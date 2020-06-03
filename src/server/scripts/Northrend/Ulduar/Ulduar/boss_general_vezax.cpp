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
SDName: General Vezax
Author: PrinceCreed, tnx Tasslehoff
SD%Complete: 100%
SDComment:
EndScriptData */

#include "ulduar.h"

enum Yells
{
    SAY_AGGRO                                   = -1603290,
    SAY_SLAY_1                                  = -1603291,
    SAY_SLAY_2                                  = -1603292,
    SAY_KITE                                    = -1603293,
    SAY_DEATH                                   = -1603294,
    SAY_BERSERK                                 = -1603295,
    SAY_HARDMODE_ON                             = -1603296,
};

#define EMOTE_VAPORS                            "A cloud of saronite vapors coalesces nearby!"
#define EMOTE_ANIMUS                            "The saronite vapors mass and swirl violently, merging into a monstrous form!"
#define EMOTE_BARRIER                           "A saronite barrier appears around General Vezax!"
#define EMOTE_DARKNESS                          "General Vezax roars and surges with dark might!"

enum VezaxSpells
{
    AURA_OF_DESPAIR                             = 62692,
    SPELL_MARK_OF_THE_FACELESS                  = 63276,
    SPELL_SARONITE_BARRIER                      = 63364,
    SPELL_SEARING_FLAMES                        = 62661,
    SPELL_SHADOW_CRASH                          = 62660,
    SPELL_SURGE_OF_DARKNESS                     = 62662,
    SPELL_SARONITE_VAPOR                        = 63323,
    SPELL_PROFOUND_OF_DARKNESS                  = 63420,
    SPELL_CORRUPTED_RAGE                        = 68415,
    SPELL_SHAMANTIC_RAGE                        = 30823,
    SPELL_SUMMON_SARONITE_ANIMUS                = 63145,
    SPELL_VISUAL_SARONITE_ANIMUS                = 63319,
    SPELL_PROFOUND_DARKNESS                     = 63420,
    SPELL_BERSERK                               = 26662,
};

enum Events
{
    EVENT_NONE,
    EVENT_SHADOW_CRASH,
    EVENT_SEARING_FLAMES,
    EVENT_DARKNESS,
    EVENT_MARK,
    EVENT_SARONITE_VAPORS,
    EVENT_BERSERK
};

enum Actions
{
    ACTION_VAPOR_DEAD,
    ACTION_ANIMUS_DEAD
};

enum Npcs
{
    NPC_SARONITE_VAPORS                         = 33488,
    BOSS_SARONITE_ANIMUS                        = 33524
};
 
#define ACHIEVEMENT_SMELL_SARONITE              RAID_MODE(3181, 3188)
#define ACHIEVEMENT_SHADOWDODGER                RAID_MODE(2996, 2997)
 
const Position VaporPos[6] =
{
{1883.89f, 125.43f, 342.378f, 0},
{1845.32f, 122.84f, 342.378f, 0},
{1788.78f, 121.50f, 342.378f, 0},
{1814.68f, 125.11f, 342.378f, 0},
{1843.22f, 155.22f, 342.378f, 0},
{1817.15f, 95.380f, 342.378f, 0}
};

const Position guardpos = {1831.62f, 74.43f, 342.27f, 1.515f};



class boss_general_vezax : public CreatureScript
{
public:
    boss_general_vezax() : CreatureScript("boss_general_vezax") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new boss_general_vezaxAI(pCreature);
    }

    struct boss_general_vezaxAI : public BossAI
    {
        boss_general_vezaxAI(Creature *pCreature) : BossAI(pCreature, BOSS_VEZAX)
        {
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
        }
        
        int32 VaporsCount;
        bool HardMode, Dodged;
        bool smellsaronite;
        
        void Reset() override
        {
            _Reset();
            //me->ResetLootMode();
            smellsaronite = false;
            VaporsCount = 0;
            HardMode = true;
            Dodged = true;
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_PROFOUND_DARKNESS);
            
            if (Creature * guard = me->FindNearestCreature(33772, 100.0f, false))
                if (guard)
                {
                    guard->DespawnOrUnsummon();
                    me->SummonCreature(33772, guardpos, TEMPSUMMON_CORPSE_DESPAWN);
                }
        }

        bool getsmellsaronite()
        {
            return smellsaronite;
        }

        bool getDodged()
        {
            return Dodged;
        }
        
        void KilledUnit(Unit* who) override
        {
            if (!(rand()%5))
                DoScriptText(RAND(SAY_SLAY_1, SAY_SLAY_2), me);
        }
    
        void SpellHitTarget(Unit * pTarget, SpellInfo const* spell) override
        {
            if (spell->Id == 62659 && pTarget->GetTypeId() == TYPEID_PLAYER)
                if (Dodged)
                {
                    me->MonsterTextEmote("Achievement Shadowdodger Fail", ObjectGuid::Empty, true);
                    Dodged = false;
                }
        }

        void EnterCombat(Unit *who) override
        {
            _EnterCombat();
            DoScriptText(SAY_AGGRO, me);
            DoCast(me, AURA_OF_DESPAIR);
            events.ScheduleEvent(EVENT_SHADOW_CRASH, 8000);
            events.ScheduleEvent(EVENT_SEARING_FLAMES, 10000);
            events.ScheduleEvent(EVENT_MARK, urand(35000, 40000));
            events.ScheduleEvent(EVENT_SARONITE_VAPORS, 30000);
            events.ScheduleEvent(EVENT_DARKNESS, 60000);
            events.ScheduleEvent(EVENT_BERSERK, 600000);
            
            if (instance)
            {
                // This ability affects Shaman with the Shamanistic Rage talent
                Map::PlayerList const &players = instance->instance->GetPlayers();
                for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                {
                    Player* pPlayer = itr->getSource();
                            
                    if (!pPlayer)
                        continue;

                    if (pPlayer->HasSpell(SPELL_SHAMANTIC_RAGE))
                        DoCast(pPlayer, SPELL_CORRUPTED_RAGE, true);
                }
            }
        }
        
        void JustDied(Unit* /*victim*/) override
        {
            DoScriptText(SAY_DEATH, me);
            _JustDied();
        }
        
        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while(uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_SHADOW_CRASH:
                        {
                            std::list<HostileReference*> ThreatList = me->getThreatManager().getThreatList();
                            for (std::list<HostileReference*>::const_iterator itr = ThreatList.begin(); itr != ThreatList.end(); ++itr)
                            {
                                if(Unit *pTarget = Unit::GetUnit(*me, (*itr)->getUnitGuid()))
                                if (me->GetDistance(pTarget) > 15.0f && pTarget->GetTypeId() == TYPEID_PLAYER)
                                {
                                    me->MonsterTextEmote("Warning Shadow Crash", ObjectGuid::Empty, true);
                                    DoCast(pTarget, SPELL_SHADOW_CRASH);
                                    break;
                                }
                            }
                            events.ScheduleEvent(EVENT_SHADOW_CRASH, urand(8000, 10000));
                            break;
                        }
                    case EVENT_SEARING_FLAMES:
                        DoCastAOE(SPELL_SEARING_FLAMES);
                        events.ScheduleEvent(EVENT_SEARING_FLAMES, urand(10000,20000));
                        break;
                    case EVENT_DARKNESS:
                        me->MonsterTextEmote(EMOTE_DARKNESS, ObjectGuid::Empty, true);
                        DoScriptText(SAY_KITE, me);
                        DoCast(me, SPELL_SURGE_OF_DARKNESS);
                        events.ScheduleEvent(EVENT_DARKNESS, urand(60000, 70000));
                        break;
                    case EVENT_MARK:
                        {
                            std::list<HostileReference*> ThreatList = me->getThreatManager().getThreatList();
                            for (std::list<HostileReference*>::const_iterator itr = ThreatList.begin(); itr != ThreatList.end(); ++itr)
                            {
                                if(Unit *pTarget = Unit::GetUnit(*me, (*itr)->getUnitGuid()))
                                {
                                    if (me->GetDistance(pTarget) > 15.0f && pTarget->GetTypeId() == TYPEID_PLAYER)
                                        if (me->getVictim())
                                            if (pTarget->GetGUID() != me->getVictim()->GetGUID())
                                            {
                                                DoCast(pTarget, SPELL_MARK_OF_THE_FACELESS);
                                                break;
                                            }
                                }
                            }
                            events.ScheduleEvent(EVENT_MARK, urand(35000, 40000));
                            break;
                        }
                    case EVENT_SARONITE_VAPORS:
                        me->MonsterTextEmote(EMOTE_VAPORS, ObjectGuid::Empty, true);
                        DoSummon(NPC_SARONITE_VAPORS, VaporPos[rand()%6], 3000, TEMPSUMMON_CORPSE_TIMED_DESPAWN);
                        events.ScheduleEvent(EVENT_SARONITE_VAPORS, 30000);
                        VaporsCount++;
                        if (VaporsCount == 6 && HardMode)
                        {
                            me->MonsterTextEmote(EMOTE_ANIMUS, ObjectGuid::Empty, true);
                            DoScriptText(SAY_HARDMODE_ON, me);
                            me->MonsterTextEmote(EMOTE_BARRIER, ObjectGuid::Empty, true);
                            DoCast(SPELL_SARONITE_BARRIER);
                            DoCast(SPELL_SUMMON_SARONITE_ANIMUS);
                            //me->AddLootMode(LOOT_MODE_HARD_MODE_1);
                            DespawnCreatures(NPC_SARONITE_VAPORS, 100);
                            events.CancelEvent(EVENT_SARONITE_VAPORS);
                            events.CancelEvent(EVENT_SEARING_FLAMES);
                        }
                        if (VaporsCount == 8)
                            events.CancelEvent(EVENT_SARONITE_VAPORS);
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
        
        void DoAction(const int32 action) override
        {
            switch (action)
            {
                case ACTION_VAPOR_DEAD:
                    HardMode = false;
                    break;
                case ACTION_ANIMUS_DEAD:
                    smellsaronite = true;
                    events.ScheduleEvent(EVENT_SEARING_FLAMES, 10000);
                    break;
                
            }
        }
    
        void DespawnCreatures(uint32 entry, float distance, bool discs = false)
        {
            std::list<Creature*> m_pCreatures;
            GetCreatureListWithEntryInGrid(m_pCreatures, me, entry, distance);
     
            if (m_pCreatures.empty())
                return;
     
            for(std::list<Creature*>::iterator iter = m_pCreatures.begin(); iter != m_pCreatures.end(); ++iter)
                (*iter)->DespawnOrUnsummon();
        }
    };
    
};


class npc_saronite_vapors : public CreatureScript
{
public:
    npc_saronite_vapors() : CreatureScript("npc_saronite_vapors") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_saronite_vaporsAI (pCreature);
    }

    struct npc_saronite_vaporsAI : public ScriptedAI
    {
        npc_saronite_vaporsAI(Creature *pCreature) : ScriptedAI(pCreature)
        {
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            me->SetReactState(REACT_PASSIVE);
            me->GetMotionMaster()->MoveRandom(30.0f);
        }
        
        void DamageTaken(Unit* /*who*/, uint32 &damage, DamageEffectType dmgType) override
        {
            if (damage >= me->GetHealth())
            {
                damage = 0;
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                me->RemoveAllAuras();
                me->SetHealth(me->GetMaxHealth());
                me->DespawnOrUnsummon(30000);
                me->GetMotionMaster()->Clear(false);
                me->GetMotionMaster()->MoveIdle();
                me->SetStandState(UNIT_STAND_STATE_DEAD);
                if (Unit* Vezax = me->ToTempSummon()->GetSummoner())
                {
                    me->CastSpell(me, SPELL_SARONITE_VAPOR, true, NULL, NULL, Vezax->GetGUID());
                    Vezax->ToCreature()->AI()->DoAction(ACTION_VAPOR_DEAD);
                }
            }
        }
    };
    
};

class npc_saronite_animus : public CreatureScript
{
public:
    npc_saronite_animus() : CreatureScript("npc_saronite_animus") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_saronite_animusAI (pCreature);
    }

    struct npc_saronite_animusAI : public ScriptedAI
    {
        npc_saronite_animusAI(Creature *pCreature) : ScriptedAI(pCreature)
        {
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);

            instance = pCreature->GetInstanceScript();
        }

        InstanceScript* instance;
        uint32 ProfoundDarknessTimer;
        
        void JustDied(Unit * killer) override
        {
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_PROFOUND_DARKNESS);

            if (Unit* Vezax = me->ToTempSummon()->GetSummoner())
            {
                Vezax->RemoveAurasDueToSpell(SPELL_SARONITE_BARRIER);
                Vezax->ToCreature()->AI()->DoAction(ACTION_ANIMUS_DEAD);
            }
        }

        void Reset() override
        {   
            DoCast(SPELL_VISUAL_SARONITE_ANIMUS);
            ProfoundDarknessTimer = 5000;
        }

        void UpdateAI(uint32 diff) override
        {
            if(!UpdateVictim())
                return;
            
            if (ProfoundDarknessTimer <= diff)
            {
                DoCastAOE(SPELL_PROFOUND_DARKNESS);
                ProfoundDarknessTimer = 5000;
            }
            else ProfoundDarknessTimer -= diff;
            
            DoMeleeAttackIfReady();
        }
    };

};

class achievement_smell_saronite : public AchievementCriteriaScript
{
    public:
        achievement_smell_saronite() : AchievementCriteriaScript("achievement_smell_saronite")
        {
        }

        bool OnCheck(Player* player, Unit* target) override
        {
            if (!target)
                return false;
             
            if (Creature * Vez = target->ToCreature())
                if (boss_general_vezax::boss_general_vezaxAI * VezAI = CAST_AI(boss_general_vezax::boss_general_vezaxAI, Vez->AI()))
                    if (VezAI->getsmellsaronite())
                        return true;
            
            return false;
        }

};

class achievement_shadowdodger : public AchievementCriteriaScript
{
    public:
        achievement_shadowdodger() : AchievementCriteriaScript("achievement_shadowdodger")
        {
        }

        bool OnCheck(Player* player, Unit* target) override
        {
            if (!target)
                return false;
             
            if (Creature * Vez = target->ToCreature())
                if (boss_general_vezax::boss_general_vezaxAI * VezAI = CAST_AI(boss_general_vezax::boss_general_vezaxAI, Vez->AI()))
                    if (VezAI->getDodged())
                        return true;
            
            return false;
        }

};


void AddSC_boss_general_vezax()
{
    new boss_general_vezax();
    new npc_saronite_vapors();
    new npc_saronite_animus();
    new achievement_smell_saronite();
    new achievement_shadowdodger();
}
