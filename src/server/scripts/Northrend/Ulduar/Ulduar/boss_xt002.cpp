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

#include "ulduar.h"
#include "Vehicle.h"

enum Spells
{
    SPELL_TYMPANIC_TANTRUM                      = 62776,
    SPELL_SEARING_LIGHT_10                      = 63018,
    SPELL_SEARING_LIGHT_25                      = 65121,
    SPELL_GRAVITY_BOMB_10                       = 63024,
    SPELL_GRAVITY_BOMB_25                       = 64234,
    SPELL_HEARTBREAK                            = 65737,
    SPELL_ENRAGE                                = 26662,

    SPELL_SUMMON_LIFE_SPARK                     = 64210,
    SPELL_SUMMON_VOID_ZONE                      = 64203,
    // Cast by 33329 on 33337 (visual?)
    SPELL_ENERGY_ORB                            = 62790,    // Triggers 62826 - needs spellscript for periodic tick to cast one of the random spells above

    // Cast by 33337 at Heartbreak:
    SPELL_RECHARGE_PUMMELER                     = 62831,    // Summons 33344
    SPELL_RECHARGE_SCRAPBOT                     = 62828,    // Summons 33343
    SPELL_RECHARGE_BOOMBOT                      = 62835,    // Summons 33346

    //------------------VOID ZONE--------------------
    SPELL_VOID_ZONE                             = 64203,
    SPELL_VOID_ZONE_DAMAGE                      = 46264,

    // Life Spark
    SPELL_STATIC_CHARGED                        = 64227,
    SPELL_SHOCK                                 = 64230,

    //----------------XT-002 HEART-------------------
    SPELL_EXPOSED_HEART                         = 63849,

    //---------------XM-024 PUMMELLER----------------
    SPELL_ARCING_SMASH                          = 8374,
    SPELL_TRAMPLE                               = 5568,
    SPELL_UPPERCUT                              = 10966,

    //------------------BOOMBOT-----------------------
    SPELL_BOOM                                  = 62834,
    
    //------------------SCRAPBOT-----------------------
    SPELL_REPAIR                                = 62832,
};

enum Events
{
    EVENT_TYMPANIC_TANTRUM = 1,
    EVENT_LIGHT_BOMB,
    EVENT_SEARING_LIGHT,
    EVENT_GRAVITY_BOMB,
    EVENT_HEART_PHASE,
    EVENT_ENERGY_ORB,
    EVENT_DISPOSE_HEART,
    EVENT_ENRAGE,
    EVENT_ENTER_HARD_MODE,
};

enum Creatures
{
    NPC_VOID_ZONE                               = 34001,
    NPC_LIFE_SPARK                              = 34004,
    NPC_XT002_HEART                             = 33329,
    NPC_XS013_SCRAPBOT                          = 33343,
    NPC_XM024_PUMMELLER                         = 33344,
    NPC_XE321_BOOMBOT                           = 33346,
};

enum Actions
{
    ACTION_ENTER_HARD_MODE                      = 0,
    ACTION_DISABLE_NERF_ACHI                    = 1,
};

enum XT002Data
{
    DATA_TRANSFERED_HEALTH,
    DATA_HARD_MODE,
    DATA_HEALTH_RECOVERED,
    DATA_GRAVITY_BOMB_CASUALTY,
};

enum Yells
{
    SAY_AGGRO                                   = -1603300,
    SAY_HEART_OPENED                            = -1603301,
    SAY_HEART_CLOSED                            = -1603302,
    SAY_TYMPANIC_TANTRUM                        = -1603303,
    SAY_SLAY_1                                  = -1603304,
    SAY_SLAY_2                                  = -1603305,
    SAY_BERSERK                                 = -1603306,
    SAY_DEATH                                   = -1603307,
    SAY_SUMMON                                  = -1603308,
};

#define EMOTE_TYMPANIC    "XT-002 Deconstructor begins to cause the earth to quake."
#define EMOTE_HEART       "XT-002 Deconstructor's heart is exposed and leaking energy."
#define EMOTE_REPAIR      "XT-002 Deconstructor consumes a scrap bot to repair himself!"

#define ACHIEV_TIMED_START_EVENT                21027
#define ACHIEVEMENT_HEARTBREAKER                RAID_MODE(3058, 3059)
#define ACHIEVEMENT_NERF_ENG                    RAID_MODE(2931, 2932)
#define ACHIEVEMENT_NERF_SCRAPBOTS              RAID_MODE(65037, 65037)

/************************************************
-----------------SPAWN LOCATIONS-----------------
************************************************/
//Shared Z-level
#define SPAWN_Z                                 412
//Lower right
#define LR_X                                    796
#define LR_Y                                    -94
//Lower left
#define LL_X                                    796
#define LL_Y                                    57
//Upper right
#define UR_X                                    890
#define UR_Y                                    -82
//Upper left
#define UL_X                                    894
#define UL_Y                                    62


/*-------------------------------------------------------
 *
 *        XT-002 DECONSTRUCTOR
 *
 *///----------------------------------------------------
class boss_xt002 : public CreatureScript
{
public:
    boss_xt002() : CreatureScript("boss_xt002") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new boss_xt002_AI(pCreature);
    }

    struct boss_xt002_AI : public BossAI
    {
        boss_xt002_AI(Creature *pCreature) : BossAI(pCreature, BOSS_XT002), vehicle(me->GetVehicleKit())
        {
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
        }

        Vehicle *vehicle;

        uint32 uiEnrageTimer;
        uint8 phase;
        uint8 HeartVal;
        bool enraged;
        bool hardMode;
        bool achievement_nerf;
        bool gbfail;

        void Reset() override
        {
            _Reset();
            me->RemoveAura(SPELL_HEARTBREAK);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_NOT_SELECTABLE);
            me->SetReactState(REACT_AGGRESSIVE);
            //me->ResetLootMode();
            me->RemoveAura(SPELL_HEARTBREAK);
            uiEnrageTimer = 600000;
            HeartVal = 0;
            enraged = false;
            hardMode = false;
            achievement_nerf = true;
            phase = 1;
            gbfail = false;
            
            if (instance)
                instance->DoStopTimedAchievement(CRITERIA_TIMED_TYPE_EVENT2, ACHIEV_TIMED_START_EVENT);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            DoScriptText(SAY_AGGRO, me);

            _EnterCombat();
            
            uiEnrageTimer = 600000;
            events.SetPhase(1);
            events.ScheduleEvent(EVENT_LIGHT_BOMB, 20000, 0, 1);
            events.ScheduleEvent(EVENT_TYMPANIC_TANTRUM, 40000, 0, 1);

            if (instance)
                instance->DoStartTimedAchievement(CRITERIA_TIMED_TYPE_EVENT2, ACHIEV_TIMED_START_EVENT);
        }

        bool isgbfail()
        {
            return gbfail;
        }

        bool ishm()
        {
            return hardMode;
        }

        void setgbfail(bool apply)
        {
            gbfail = apply;
        }

        void DoAction(const int32 action) override
        {
            switch (action)
            {
                case ACTION_ENTER_HARD_MODE:
                    if (!hardMode)
                    {
                        hardMode = true;
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_STUNNED);
                        me->SetReactState(REACT_AGGRESSIVE);
                        DoZoneInCombat();
                        //me->AddLootMode(LOOT_MODE_HARD_MODE_1);
                        me->SetFullHealth();
                        DoCast(me, SPELL_HEARTBREAK, true);
                        events.SetPhase(1);
                        phase = 1;
                        events.ScheduleEvent(EVENT_LIGHT_BOMB, 10000, 0, 1);
                        events.ScheduleEvent(EVENT_TYMPANIC_TANTRUM, 20000, 0, 1);
                    }
                    break;
            }
        }
        
        void KilledUnit(Unit* who) override
        {
            DoScriptText(RAND(SAY_SLAY_1,SAY_SLAY_2), me);
        }

        void SpellHit(Unit * caster, SpellInfo const* spell) override
        {
            if (achievement_nerf)
            {
                if (spell->Id == SPELL_REPAIR)
                    achievement_nerf = false;
            }
        }

        void JustDied(Unit* /*victim*/) override
        {
            DoScriptText(SAY_DEATH, me);
            _JustDied();

            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);

            if (instance)
            {
                // Heartbreaker
                if (hardMode)
                    instance->DoCompleteAchievement(ACHIEVEMENT_HEARTBREAKER);
                // Nerf Engineering
                if (achievement_nerf)
                    instance->DoCompleteAchievement(ACHIEVEMENT_NERF_ENG);
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            if (uiEnrageTimer <= diff)
            {
                if (!enraged)
                {
                    if (hardMode)
                    {
                        DoScriptText(SAY_BERSERK, me);
                        DoCast(me, SPELL_ENRAGE);
                        enraged = true;
                    }
                } 
                
            } else uiEnrageTimer -= diff;
            
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
            
            if (HealthBelowPct(75) && !HeartVal && !hardMode && phase != 2)
            {
                HeartVal++;
                events.SetPhase(2);
                phase = 2;
                events.ScheduleEvent(EVENT_HEART_PHASE, 35000, 0, 2);
                exposeHeart();
            }
            else if (HealthBelowPct(50) && HeartVal == 1 && !hardMode && phase != 2)
            {
                HeartVal++;
                events.SetPhase(2);
                phase = 2;
                events.ScheduleEvent(EVENT_HEART_PHASE, 35000, 0, 2);
                exposeHeart();
            }
            else if (HealthBelowPct(25) && HeartVal == 2 && !hardMode && phase != 2)
            {
                HeartVal++;
                events.SetPhase(2);
                phase = 2;
                events.ScheduleEvent(EVENT_HEART_PHASE, 35000, 0, 2);
                exposeHeart();
            }
            
            events.Update(diff);

            while (uint32 eventid = events.ExecuteEvent())
            {
                switch(eventid)
                {
                    case EVENT_LIGHT_BOMB:
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                        {
                            if (urand(0, 1))
                                DoCast(target, RAID_MODE(SPELL_GRAVITY_BOMB_10, SPELL_GRAVITY_BOMB_25));
                            else
                                DoCast(target, RAID_MODE(SPELL_SEARING_LIGHT_10, SPELL_SEARING_LIGHT_25));
                        }

                        events.ScheduleEvent(EVENT_LIGHT_BOMB, 20000, 0, 1); 
                    }
                    break;
                    case EVENT_TYMPANIC_TANTRUM:
                        DoScriptText(SAY_TYMPANIC_TANTRUM, me);
                        DoCast(SPELL_TYMPANIC_TANTRUM);
                        events.ScheduleEvent(EVENT_TYMPANIC_TANTRUM, 60000, 0, 1);
                        break;
                    case EVENT_HEART_PHASE:
                        DoScriptText(SAY_HEART_CLOSED, me);
                        me->HandleEmoteCommand(EMOTE_ONESHOT_EMERGE);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_STUNNED);
                        me->SetReactState(REACT_AGGRESSIVE);
                        me->GetMotionMaster()->Clear();
                        SetCombatMovement(true);
                        DoZoneInCombat();
                        events.SetPhase(1);
                        phase = 1;
                        events.CancelEvent(EVENT_HEART_PHASE);
                        events.ScheduleEvent(EVENT_LIGHT_BOMB, 20000, 0, 1);
                        events.ScheduleEvent(EVENT_TYMPANIC_TANTRUM, 40000, 0, 1);
                        break;
                }
            }
            
            if (phase == 1)
                DoMeleeAttackIfReady();
        }

        void exposeHeart()
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_STUNNED);
            me->SetReactState(REACT_PASSIVE);
            me->GetMotionMaster()->Clear(false);
            me->GetMotionMaster()->MoveIdle();
            me->AttackStop();
            SetCombatMovement(false);
            if (Unit *Heart = vehicle->GetPassenger(0))
                Heart->ToCreature()->AI()->DoAction(0);

            DoScriptText(SAY_HEART_OPENED, me);
            me->MonsterTextEmote(EMOTE_HEART, ObjectGuid::Empty, true);
            me->HandleEmoteCommand(EMOTE_ONESHOT_SUBMERGE);
            SpawnAdds();
        }
        
        void SpawnAdds()
        {
            switch (rand() % 4)
            {
                case 0: me->SummonCreature(NPC_XM024_PUMMELLER, LR_X, LR_Y, SPAWN_Z, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000); break;
                case 1: me->SummonCreature(NPC_XM024_PUMMELLER, LL_X, LL_Y, SPAWN_Z, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000); break;
                case 2: me->SummonCreature(NPC_XM024_PUMMELLER, UR_X, UR_Y, SPAWN_Z, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000); break;
                case 3: me->SummonCreature(NPC_XM024_PUMMELLER, UL_X, UL_Y, SPAWN_Z, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000); break;
            }

            for (uint32 i = 0; i < urand(7, 10); i++)
            {
                switch(rand() % 4)
                {
                case 0: 
                    for (int8 n = 0; n < 5; n++)
                        me->SummonCreature(NPC_XS013_SCRAPBOT, irand(LR_X - 3, LR_X + 3), irand(LR_Y - 3, LR_Y + 3), SPAWN_Z, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000); break;
                case 1: 
                    for (int8 n = 0; n < 5; n++)
                        me->SummonCreature(NPC_XS013_SCRAPBOT, irand(LL_X - 3, LL_X + 3), irand(LL_Y - 3, LL_Y + 3), SPAWN_Z, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000); break;
                case 2: 
                    for (int8 n = 0; n < 5; n++)
                        me->SummonCreature(NPC_XS013_SCRAPBOT, irand(UR_X - 3, UR_X + 3), irand(UR_Y - 3, UR_Y + 3), SPAWN_Z, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000); break;
                case 3: 
                    for (int8 n = 0; n < 5; n++)
                        me->SummonCreature(NPC_XS013_SCRAPBOT, irand(UL_X - 3, UL_X + 3), irand(UL_Y - 3, UL_Y + 3), SPAWN_Z, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000); break;
                }
            }
            switch (rand() % 4)
            {
            case 0: 
                me->SummonCreature(NPC_XE321_BOOMBOT, LR_X, LR_Y, SPAWN_Z, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                me->SummonCreature(NPC_XE321_BOOMBOT, LR_X, LR_Y, SPAWN_Z, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                break;
            case 1: 
                me->SummonCreature(NPC_XE321_BOOMBOT, LL_X, LL_Y, SPAWN_Z, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                me->SummonCreature(NPC_XE321_BOOMBOT, LL_X, LL_Y, SPAWN_Z, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000); 
                break;
            case 2: 
                me->SummonCreature(NPC_XE321_BOOMBOT, UR_X, UR_Y, SPAWN_Z, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000); 
                me->SummonCreature(NPC_XE321_BOOMBOT, UR_X, UR_Y, SPAWN_Z, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                break;
            case 3: 
                me->SummonCreature(NPC_XE321_BOOMBOT, UL_X, UL_Y, SPAWN_Z, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                me->SummonCreature(NPC_XE321_BOOMBOT, UL_X, UL_Y, SPAWN_Z, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000); 
                break;
            }
        };

    };
};
/*-------------------------------------------------------
 *
 *        XT-002 HEART
 *
 *///----------------------------------------------------
class mob_xt002_heart : public CreatureScript
{
public:
    mob_xt002_heart() : CreatureScript("mob_xt002_heart") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new mob_xt002_heartAI(pCreature);
    }

    struct mob_xt002_heartAI : public ScriptedAI
    {
        mob_xt002_heartAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
        }

        uint32 uiExposeTimer;
        bool Exposed;

        void JustDied(Unit* /*victim*/) override
        {
            if (Unit* pXT002 = me->ToTempSummon()->GetSummoner())
                pXT002->ToCreature()->AI()->DoAction(ACTION_ENTER_HARD_MODE);

            me->DespawnOrUnsummon();
        }

        void UpdateAI(uint32 diff) override
        {
            if (Exposed)
            {
                if (!me->HasAura(SPELL_EXPOSED_HEART))
                {
                    Exposed = false;
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    me->RemoveAllAuras();
                    me->SetFullHealth();
                    me->ChangeSeat(0);
                    
                }
            }
            else
            {
                if (!me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
                {
                    if (uiExposeTimer <= diff)
                    {
                        Exposed = true;
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                        DoCast(me, SPELL_EXPOSED_HEART, true);
                    }
                    else uiExposeTimer -= diff;
                }
            }
        }

        void Reset() override
        {
            Exposed = false;
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        }

        void DamageTaken(Unit *pDone, uint32 &damage, DamageEffectType dmgType) override
        {
            if (Unit *pXT002 = me->ToTempSummon()->GetSummoner())
            {
                if (damage > me->GetHealth())
                    damage = me->GetHealth();
                
                if (pDone)
                    pDone->DealDamage(pXT002, damage);
            }
        }

        void DoAction(const int32 action) override
        {
            if (action == 0)
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->ChangeSeat(1);
                uiExposeTimer = 3500;
            }
        }
    };

};

/*-------------------------------------------------------
 *
 *        XS-013 SCRAPBOT
 *
 *///----------------------------------------------------
class mob_scrapbot : public CreatureScript
{
public:
    mob_scrapbot() : CreatureScript("mob_scrapbot") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new mob_scrapbotAI(pCreature);
    }

    struct mob_scrapbotAI : public ScriptedAI
    {
        mob_scrapbotAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            m_pInstance = me->GetInstanceScript();
        }

        InstanceScript* m_pInstance;
        bool repaired;
        

        void Reset() override
        {
            me->SetReactState(REACT_PASSIVE);
            repaired = false;
            

            if (Creature* pXT002 = me->GetCreature(*me, m_pInstance->GetGuidData(DATA_XT002)))
                me->GetMotionMaster()->MoveFollow(pXT002, 0.0f, 0.0f);
        }

        void UpdateAI(uint32 diff) override
        {
            if (Creature* pXT002 = me->GetCreature(*me, m_pInstance->GetGuidData(DATA_XT002)))
            {
                if (!repaired && me->GetDistance2d(pXT002) <= 0.5)
                {
                    me->MonsterTextEmote(EMOTE_REPAIR, ObjectGuid::Empty, true);
                   
                    pXT002->CastSpell(me, SPELL_REPAIR, true);
                    repaired = true;
                    me->DespawnOrUnsummon(1000);
                }
            }
        }
        
        void JustDied(Unit * killer) override
        {
            if (killer && killer->GetTypeId() == TYPEID_UNIT && killer->GetEntry() == NPC_XE321_BOOMBOT)
            {
                if (m_pInstance)
                {
                    m_pInstance->DoStartTimedAchievement(CRITERIA_TIMED_TYPE_SPELL_TARGET2, ACHIEVEMENT_NERF_SCRAPBOTS);
                    m_pInstance->DoUpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET, ACHIEVEMENT_NERF_SCRAPBOTS);
                }
                    
            }
        }
    };

};


/*-------------------------------------------------------
 *
 *        XM-024 PUMMELLER
 *
 *///----------------------------------------------------
class mob_pummeller : public CreatureScript
{
public:
    mob_pummeller() : CreatureScript("mob_pummeller") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new mob_pummellerAI(pCreature);
    }

    struct mob_pummellerAI : public ScriptedAI
    {
        mob_pummellerAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            m_pInstance = pCreature->GetInstanceScript();
        }

        InstanceScript* m_pInstance;
        uint32 uiArcingSmashTimer;
        uint32 uiTrampleTimer;
        uint32 uiUppercutTimer;

        void Reset() override
        {
            uiArcingSmashTimer = 27000;
            uiTrampleTimer = 22000;
            uiUppercutTimer = 17000;
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            if (me->IsWithinMeleeRange(me->getVictim()))
            {
                if (uiArcingSmashTimer <= diff)
                {
                    DoCast(me->getVictim(), SPELL_ARCING_SMASH);
                    uiArcingSmashTimer = 27000;
                } else uiArcingSmashTimer -= diff;

                if (uiTrampleTimer <= diff)
                {
                    DoCast(me->getVictim(), SPELL_TRAMPLE);
                    uiTrampleTimer = 22000;
                } else uiTrampleTimer -= diff;

                if (uiUppercutTimer <= diff)
                {
                    DoCast(me->getVictim(), SPELL_UPPERCUT);
                    uiUppercutTimer = 17000;
                } else uiUppercutTimer -= diff;
            }

            DoMeleeAttackIfReady();
        }
    };

};


/*-------------------------------------------------------
 *
 *        XE-321 BOOMBOT
 *
 *///----------------------------------------------------
class mob_boombot : public CreatureScript
{
public:
    mob_boombot() : CreatureScript("mob_boombot") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new mob_boombotAI(pCreature);
    }

    struct mob_boombotAI : public ScriptedAI
    {
        mob_boombotAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            m_pInstance = pCreature->GetInstanceScript();
        }

        InstanceScript* m_pInstance;

        void Reset() override
        {
            if (Creature* pXT002 = me->GetCreature(*me, m_pInstance->GetGuidData(DATA_XT002)))
                me->GetMotionMaster()->MoveFollow(pXT002, 0.0f, 0.0f);
        }

        void UpdateAI(uint32 diff) override
        {
            if (me->IsWithinMeleeRange(me->getVictim()))
            {
                if (me->GetDistance(me->getVictim()) <= 0.5f || HealthBelowPct(50))
                    DoCast(me, SPELL_BOOM);
            }
                
        }
    };
};


/*-------------------------------------------------------
 *
 *        VOID ZONE
 *
 *///----------------------------------------------------
class mob_void_zone : public CreatureScript
{
public:
    mob_void_zone() : CreatureScript("mob_void_zone") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new mob_void_zoneAI(pCreature);
    }

    struct mob_void_zoneAI : public Scripted_NoMovementAI
    {
        mob_void_zoneAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
        {
            m_pInstance = pCreature->GetInstanceScript();
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
        }

        InstanceScript* m_pInstance;
        uint32 uiVoidZoneTimer;

        void Reset() override
        {
            uiVoidZoneTimer = 2000;
        }

        void UpdateAI(uint32 diff) override
        {
            if (uiVoidZoneTimer <= diff)
            {
                DoCast(SPELL_VOID_ZONE_DAMAGE);
                uiVoidZoneTimer = 2000;
            } else uiVoidZoneTimer -= diff;
        }
    };

};


/*-------------------------------------------------------
 *
 *        LIFE SPARK
 *
 *///----------------------------------------------------
class mob_life_spark : public CreatureScript
{
public:
    mob_life_spark() : CreatureScript("mob_life_spark") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new mob_life_sparkAI(pCreature);
    }

    struct mob_life_sparkAI : public ScriptedAI
    {
        mob_life_sparkAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            m_pInstance = pCreature->GetInstanceScript();
        }

        InstanceScript* m_pInstance;
        uint32 uiShockTimer;

        void Reset() override
        {
            DoCast(me, SPELL_STATIC_CHARGED);
            uiShockTimer = 0; // first one is immediate.
        }

        void UpdateAI(uint32 diff) override
        {
            if (m_pInstance && m_pInstance->GetBossState(BOSS_XT002) != IN_PROGRESS)
                me->DespawnOrUnsummon();

            if (uiShockTimer <= diff)
            {
                if (me->IsWithinMeleeRange(me->getVictim()))
                {
                    DoCast(me->getVictim(), SPELL_SHOCK);
                    uiShockTimer = 12000;
                }
            }
            else uiShockTimer -= diff;
        }
    };

};

class spell_xt002_searing_light_spawn_life_spark : public SpellScriptLoader
{
    public:
        spell_xt002_searing_light_spawn_life_spark() : SpellScriptLoader("spell_xt002_searing_light_spawn_life_spark") { }

        class spell_xt002_searing_light_spawn_life_spark_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_xt002_searing_light_spawn_life_spark_AuraScript);

            bool Validate(SpellInfo const* /*spell*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_SUMMON_LIFE_SPARK))
                    return false;
                return true;
            }

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Player* player = GetOwner()->ToPlayer())
                    if (Creature* xt002c = GetCaster()->ToCreature())
                        if (boss_xt002::boss_xt002_AI* xt002cAI = CAST_AI(boss_xt002::boss_xt002_AI, xt002c->AI()))
                            if (xt002cAI->ishm())
                                player->CastSpell(player, SPELL_SUMMON_LIFE_SPARK, true);
            }

            void Register() override
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_xt002_searing_light_spawn_life_spark_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_xt002_searing_light_spawn_life_spark_AuraScript();
        }
};

class spell_xt002_gravity_bomb_aura : public SpellScriptLoader
{
    public:
        spell_xt002_gravity_bomb_aura() : SpellScriptLoader("spell_xt002_gravity_bomb_aura") { }

        class spell_xt002_gravity_bomb_aura_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_xt002_gravity_bomb_aura_AuraScript);

            bool Validate(SpellInfo const* /*spell*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_SUMMON_VOID_ZONE))
                    return false;
                return true;
            }

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Player* player = GetOwner()->ToPlayer())
                    if (Creature* xt002c = GetCaster()->ToCreature())
                        if (boss_xt002::boss_xt002_AI* xt002cAI = CAST_AI(boss_xt002::boss_xt002_AI, xt002c->AI()))
                            if (xt002cAI->ishm())
                                player->CastSpell(player, SPELL_SUMMON_VOID_ZONE, true);
            }

            void OnPeriodic(AuraEffect const* aurEff)
            {
                Unit* xt002 = GetCaster();
                if (!xt002)
                    return;

                Unit* owner = GetOwner()->ToUnit();
                if (!owner)
                    return;

                Creature * xt002c = xt002->ToCreature();

                if (!xt002c)
                    return;

                if (aurEff->GetAmount() >= int32(owner->GetHealth()))
                    if (boss_xt002::boss_xt002_AI* xt002cAI = CAST_AI(boss_xt002::boss_xt002_AI, xt002c->AI()))
                        xt002cAI->setgbfail(true);
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_xt002_gravity_bomb_aura_AuraScript::OnPeriodic, EFFECT_2, SPELL_AURA_PERIODIC_DAMAGE);
                AfterEffectRemove += AuraEffectRemoveFn(spell_xt002_gravity_bomb_aura_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_xt002_gravity_bomb_aura_AuraScript();
        }
};

class spell_xt002_gravity_bomb_damage : public SpellScriptLoader
{
    public:
        spell_xt002_gravity_bomb_damage() : SpellScriptLoader("spell_xt002_gravity_bomb_damage") { }

        class spell_xt002_gravity_bomb_damage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_xt002_gravity_bomb_damage_SpellScript);

            void HandleScript(SpellEffIndex /*eff*/)
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                Creature * xt002c = caster->ToCreature();

                if (!xt002c)
                    return;

                if (GetHitDamage() >= int32(GetHitUnit()->GetHealth()))
                    if (boss_xt002::boss_xt002_AI* xt002cAI = CAST_AI(boss_xt002::boss_xt002_AI, xt002c->AI()))
                        xt002cAI->setgbfail(true);
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_xt002_gravity_bomb_damage_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_xt002_gravity_bomb_damage_SpellScript();
        }
};

class spell_xt002_heart_overload_periodic : public SpellScriptLoader
{
    public:
        spell_xt002_heart_overload_periodic() : SpellScriptLoader("spell_xt002_heart_overload_periodic") { }

        class spell_xt002_heart_overload_periodic_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_xt002_heart_overload_periodic_SpellScript);

            bool Validate(SpellInfo const* /*spell*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_ENERGY_ORB))
                    return false;

                if (!sSpellMgr->GetSpellInfo(SPELL_RECHARGE_BOOMBOT))
                    return false;

                if (!sSpellMgr->GetSpellInfo(SPELL_RECHARGE_PUMMELER))
                    return false;

                if (!sSpellMgr->GetSpellInfo(SPELL_RECHARGE_SCRAPBOT))
                    return false;

                return true;
            }

            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (InstanceScript* instance = caster->GetInstanceScript())
                    {
                        if (Unit* toyPile = ObjectAccessor::GetUnit(*caster, instance->GetGuidData(DATA_TOY_PILE_0 + urand(0, 3))))
                        {
                            caster->CastSpell(toyPile, SPELL_ENERGY_ORB, true);

                            // This should probably be incorporated in a dummy effect handler, but I've had trouble getting the correct target
                            // Weighed randomization (approximation)
                            uint32 const spells[] = { SPELL_RECHARGE_SCRAPBOT, SPELL_RECHARGE_SCRAPBOT, SPELL_RECHARGE_SCRAPBOT,
                                SPELL_RECHARGE_PUMMELER, SPELL_RECHARGE_BOOMBOT };

                            for (uint8 i = 0; i < 5; ++i)
                            {
                                uint8 a = urand(0, 4);
                                uint32 spellId = spells[a];
                                toyPile->CastSpell(toyPile, spellId, true, NULL, NULL, instance->GetGuidData(BOSS_XT002));
                            }
                        }
                    }

                    DoScriptText(SAY_SUMMON, caster->GetVehicleBase());
                }
            }

            void Register() override
            {
                OnEffectHit += SpellEffectFn(spell_xt002_heart_overload_periodic_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_xt002_heart_overload_periodic_SpellScript();
        }
};

class spell_xt002_tympanic_tantrum : public SpellScriptLoader
{
    public:
        spell_xt002_tympanic_tantrum() : SpellScriptLoader("spell_xt002_tympanic_tantrum") { }

        class spell_xt002_tympanic_tantrum_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_xt002_tympanic_tantrum_SpellScript);

            void FilterTargets(std::list<WorldObject*>& unitList)
            {
                unitList.remove_if(PlayerOrPetCheck());
            }

            void RecalculateDamage()
            {
                SetHitDamage(GetHitUnit()->CountPctFromMaxHealth(GetHitDamage()));
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_xt002_tympanic_tantrum_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_xt002_tympanic_tantrum_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
                OnHit += SpellHitFn(spell_xt002_tympanic_tantrum_SpellScript::RecalculateDamage);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_xt002_tympanic_tantrum_SpellScript();
        }
};

class spell_xt002_submerged : public SpellScriptLoader
{
    public:
        spell_xt002_submerged() : SpellScriptLoader("spell_xt002_submerged") { }

        class spell_xt002_submerged_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_xt002_submerged_SpellScript);

            void HandleScript(SpellEffIndex /*eff*/)
            {
                Creature* target = GetHitCreature();
                if (!target)
                    return;

                target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                target->SetByteValue(UNIT_FIELD_BYTES_1, 0, UNIT_STAND_STATE_SUBMERGED);
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_xt002_submerged_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_xt002_submerged_SpellScript();
        }
};

class spell_xt002_stand : public SpellScriptLoader
{
    public:
        spell_xt002_stand() : SpellScriptLoader("spell_xt002_stand") { }

        class spell_xt002_stand_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_xt002_stand_SpellScript);

            void HandleScript(SpellEffIndex /*eff*/)
            {
                Creature* target = GetHitCreature();
                if (!target)
                    return;

                target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                target->SetByteValue(UNIT_FIELD_BYTES_1, 0, UNIT_STAND_STATE_STAND);
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_xt002_stand_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_xt002_stand_SpellScript();
        }
};

class achievement_nerf_gravity_bombs : public AchievementCriteriaScript
{
    public:
        achievement_nerf_gravity_bombs() : AchievementCriteriaScript("achievement_nerf_gravity_bombs")
        {
        }

        bool OnCheck(Player* player, Unit* target) override
        {
            if (!target)
                return false;

            if (target->GetEntry() != NPC_XT002)
                return false;

            if (Creature * xt002c = target->ToCreature())
                if (boss_xt002::boss_xt002_AI* xt002cAI = CAST_AI(boss_xt002::boss_xt002_AI, xt002c->AI()))
                    if (!xt002cAI->isgbfail())
                        return true;

            return false;
        }
};

void AddSC_boss_xt002()
{
    new mob_xt002_heart();
    new mob_scrapbot();
    new mob_pummeller();
    new mob_boombot();
    new mob_void_zone();
    new mob_life_spark();
    new boss_xt002();
    new spell_xt002_searing_light_spawn_life_spark();
    new spell_xt002_gravity_bomb_aura();
    new spell_xt002_gravity_bomb_damage();
    //new spell_xt002_heart_overload_periodic();
    new spell_xt002_tympanic_tantrum();
    new spell_xt002_submerged();
    new spell_xt002_stand();;
    new achievement_nerf_gravity_bombs();

    if (VehicleSeatEntry* vehSeat = const_cast<VehicleSeatEntry*>(sVehicleSeatStore.LookupEntry(3846)))
        vehSeat->Flags |= 0x400;
}
