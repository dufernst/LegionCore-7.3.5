/*
* Copyright (C) 2009 - 2010 TrinityCore <http://www.trinitycore.org/>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "icecrown_citadel.h"
#include "ObjectVisitors.hpp"

enum Texts
{
    // The Lich King
    SAY_LICH_KING_INTRO         = 0,

    // Valithria Dreamwalker
    SAY_VALITHRIA_ENTER_COMBAT  = 0,
    SAY_VALITHRIA_DREAM_PORTAL  = 1,
    SAY_VALITHRIA_75_PERCENT    = 2,
    SAY_VALITHRIA_25_PERCENT    = 3,
    SAY_VALITHRIA_DEATH         = 4,
    SAY_VALITHRIA_PLAYER_DEATH  = 5,
    SAY_VALITHRIA_BERSERK       = 6,
    SAY_VALITHRIA_SUCCESS       = 7,
};

enum Spells
{
    SPELL_CORRUPTION    = 70904,
    SPELL_DREAM_SLIP    = 71196,
    SPELL_RAGE          = 71189,
    SPELL_VOLLEY        = 70759,
    SPELL_COLUMN_OF_FROST    = 70704,
    SPELL_COLUMN_EFFECT_10N = 70702,
    SPELL_COLUMN_EFFECT_10H_25N = 71746,
    SPELL_COLUMN_EFFECT_25H = 72020,
    SPELL_MANA_VOID     = 71085,
    SPELL_CORRUPTING    = 70602,
    SPELL_WASTE         = 69325,
    SPELL_FIREBALL      = 70754,
    SPELL_SUPRESSION    = 70588,
    SPELL_CORROSION     = 70751,
    SPELL_BURST         = 70744,
    SPELL_SPRAY         = 71283,
    SPELL_ROT           = 72963,
    SPELL_DREAM_STATE   = 70766,
    SPELL_DREAM_PORTAL_VISUAL_PRE = 71304,
    SPELL_CLOUD_VISUAL  = 70876,
    SPELL_NIGHTMARE_PORTAL_VISUAL_PRE  = 71986,
    SPELL_N_PORTAL_V    = 71994,
    SPELL_PORTAL_N_PRE  = 71301,
    SPELL_SUMMON_DREAM_PORTAL  = 71305,
    SPELL_PORTAL_H_PRE  = 71977,
    SPELL_SUMMON_NIGHTMARE_PORTAL  = 71987,
    SPELL_NIGHTMARE                     = 71941,//iaiiu iia ia?nii
    SPELL_MOD_DAMAGE                    = 68066,
    SPELL_COPY_DAMAGE                   = 71948,
    SPELL_SUMMON_SUPPRESSOR_PEREODIC    = 70912,
    SPELL_TIMER_BLAZING_SKELETON        = 70913,
    SPELL_SUMMON_ZOMBIE_PEREODIC        = 70914,
    SPELL_SUMMON_ABOMINATION_PEREODIC   = 70915,
    SPELL_SUMMON_ARCHMAGE_PEREODIC      = 70916,
    SPELL_SUMMON_SUPPRESSOR             = 70935,
    SPEEL_CLEAR_AURA                    = 75863,
    SPELL_CANCEL_ALL_AURAS              = 71721,
    SPELL_AWARD_REPUTATION_BOSS_KILL    = 73843,
    SPELL_ACHIEVEMENT_CHECK             = 72706,
    SPELL_NIGHTMARE_DAMAGE              = 71946,
    SPELL_PRE_SUMMON_DREAM_PORTAL       = 72224,
    SPELL_PRE_SUMMON_NIGHTMARE_PORTAL   = 72480,
    SPELL_RECENTLY_SPAWNED              = 72954,
    SPELL_SUMMON_SUPPRESSER             = 70936,

    // Dream Cloud
    SPELL_EMERALD_VIGOR                 = 70873,

    // Nightmare Cloud
    SPELL_TWISTED_NIGHTMARE             = 71941,
};

#define SUMMON_PORTAL RAID_MODE<uint32>(SPELL_PRE_SUMMON_DREAM_PORTAL, SPELL_PRE_SUMMON_DREAM_PORTAL, \
                                        SPELL_PRE_SUMMON_NIGHTMARE_PORTAL, SPELL_PRE_SUMMON_NIGHTMARE_PORTAL)

#define EMERALD_VIGOR RAID_MODE<uint32>(SPELL_EMERALD_VIGOR, SPELL_EMERALD_VIGOR, \
                                        SPELL_TWISTED_NIGHTMARE, SPELL_TWISTED_NIGHTMARE)


enum eEvents
{
    EVENT_SUMMON_DREAM_CLOUD        = 1,
    EVENT_DESPAWN                   = 3,
    EVENT_DESPAWN_AND_FAIL_ACHIEVEMENT = 20,
    //Events played when boss is healed up to 100%
    EVENT_BERSERK                   = 4,
    EVENT_EVADE_TO_DREAM_SLIP       = 5,

    //Adds
    EVENT_SUMMON_ARCHMAGE           = 8,
    EVENT_SUMMON_ZOMBIE             = 17,
    EVENT_SUMMON_ABOMINATION        = 18,
    EVENT_SUMMON_SUPPRESSOR          = 9,
    EVENT_SUMMON_BLAZING_SKELETON   = 10,
    EVENT_CAST_COLUMN_OF_FROST      = 11,
    EVENT_HASTEN_SUMMON_TIMER       = 12,

    //Portals
    EVENT_SUMMON_PORTALS_TO_DREAM   = 2,
    EVENT_DREAM_PORTAL              = 6,
    EVENT_EXPLODE                   = 13,
    EVENT_CLOUD_EMULATE_DESPAWN     = 14,
    EVENT_CLOUD_EMULATE_RESPAWN     = 15,
    EVENT_INTRO                     = 7,

    EVENT_CHECK_WIPE                = 16,
    EVENT_CAST_ROT_WORM_SPAWN_ANIMATION = 19,

    EVENT_GUT_SPRAY                 = 21,
    EVENT_ENABLE_CASTING            = 22, 
    EVENT_FROSTBOLT_VOLLEY          = 23,
    EVENT_SUMMON_MANA_VOID          = 24,
    EVENT_CAST_SUPPRESSION          = 25,

    EVENT_CAST_FIREBALL             = 26,
    EVENT_CAST_LAY_WASTE            = 27,
    EVENT_CHECK_EVADE               = 28,

    // Dream Cloud
    // Nightmare Cloud
    EVENT_CHECK_PLAYER              = 29,
};

enum eEnums
{
    MODEL_INVISIBLE                       = 11686,
};

enum Actions
{
    ACTION_ENTER_COMBAT = 1,
    MISSED_PORTALS      = 2,
    ACTION_DEATH        = 3,
};

const Position Pos[] =
{
    {4239.579102f, 2566.753418f, 364.868439f, 0.0f}, //normal 0,1
    {4240.688477f, 2405.794678f, 364.868591f, 0.0f}, // normal
    {4165.112305f, 2405.872559f, 364.872925f, 0.0f}, //2,3
    {4166.216797f, 2564.197266f, 364.873047f, 0.0f}
};

Position const ValithriaSpawnPos = {4210.813f, 2484.443f, 364.9558f, 0.01745329f};

struct ManaVoidSelector : public std::unary_function<Unit*, bool>
{
        explicit ManaVoidSelector(WorldObject const* source) : _source(source) { }

        bool operator()(Unit* unit) const
        {
            return unit->getPowerType() == POWER_MANA && _source->GetDistance(unit) > 15.0f;
        }

        WorldObject const* _source;
};

class DelayedCastEvent : public BasicEvent
{
    public:
        DelayedCastEvent(Creature* trigger, uint32 spellId, ObjectGuid originalCaster, uint32 despawnTime) : _trigger(trigger), _originalCaster(originalCaster), _spellId(spellId), _despawnTime(despawnTime)
        {
        }

        bool Execute(uint64 /*time*/, uint32 /*diff*/) override
        {
            _trigger->CastSpell(_trigger, _spellId, false, NULL, NULL, _originalCaster);
            if (_despawnTime)
                _trigger->DespawnOrUnsummon(_despawnTime);
            return true;
        }

    private:
        Creature* _trigger;
        ObjectGuid _originalCaster;
        uint32 _spellId;
        uint32 _despawnTime;
};

class AuraRemoveEvent : public BasicEvent
{
    public:
        AuraRemoveEvent(Creature* trigger, uint32 spellId) : _trigger(trigger), _spellId(spellId)
        {
        }

        bool Execute(uint64 /*time*/, uint32 /*diff*/) override
        {
            _trigger->RemoveAurasDueToSpell(_spellId);
            return true;
        }

    private:
        Creature* _trigger;
        uint32 _spellId;
};

class boss_valithria_dreamwalker : public CreatureScript
{
    public:
        boss_valithria_dreamwalker() : CreatureScript("boss_valithria_dreamwalker") { }

        struct boss_valithria_dreamwalkerAI : public BossAI
        {
            boss_valithria_dreamwalkerAI(Creature* pCreature) : BossAI(pCreature, DATA_VALITHRIA_DREAMWALKER),
            summons(me), _portalCount(RAID_MODE<uint32>(3, 7, 3, 7))
            {
                instance = me->GetInstanceScript();
                despawn = false;
            }

            void Reset() override
            {
                if (despawn)
                    return;

                _Reset();
                events.Reset();
                m_uiStage = 1;

                DoCast(me, SPELL_CORRUPTION);
                me->SetHealth(uint32(me->GetMaxHealth() * 0.50));
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_OBS_MOD_HEALTH, true);
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_HEAL_PCT, true);
                // Glyph of Dispel Magic - not a percent heal by effect, its cast with custom basepoints
                me->ApplySpellImmune(0, IMMUNITY_ID, 56131, true);

                m_uiSummonTimer = 30000;
                m_uiPortalTimer = 0;
                m_uiEndTimer = 1000;
                _missedPortals = 0;
                _openPortals = 0;

                bIntro = false;
                bEnd = false;
                bAboveHP = false;
                bBelowHP = false;

                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                
                //if (instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE)
                    //me->setFaction(2144);

                DoCast(me, SPELL_COPY_DAMAGE);

                if (instance && me->isAlive())
                    instance->SetBossState(DATA_VALITHRIA_DREAMWALKER, NOT_STARTED);

                summons.DespawnAll();
            }

            bool HealthAbovePct(int32 pct) const { return me->GetHealth() * (uint64)100 > me->GetMaxHealth() * (uint64)pct; }

            bool HealthBelowPct(int32 pct) const { return me->GetHealth() * (uint64)100 < me->GetMaxHealth() * (uint64)pct; }

            void MoveInLineOfSight(Unit* who) override
            {
                if (despawn)
                    return;

                if (instance && !bIntro && who->IsPlayer() && who->IsWithinDistInMap(me, 3.0f,true))
                {
                    if (instance && instance->GetBossState(DATA_VALITHRIA_DREAMWALKER) == NOT_STARTED)
                        instance->SetBossState(DATA_VALITHRIA_DREAMWALKER, IN_PROGRESS);

                    Talk(SAY_VALITHRIA_ENTER_COMBAT);
                    bIntro = true;
                    me->SetHealth(uint32(me->GetMaxHealth() * 0.50));

                    m_uiSummonSkeletonTimer = 60000;
                    m_uiSummonSuppressorTimer = 60000;
                    events.Reset();
                    events.ScheduleEvent(EVENT_SUMMON_ZOMBIE, 17000);
                    events.ScheduleEvent(EVENT_SUMMON_ARCHMAGE, 2000);
                    events.ScheduleEvent(EVENT_SUMMON_ABOMINATION, 10000);
                    events.ScheduleEvent(EVENT_SUMMON_SUPPRESSOR, 20000);
                    events.ScheduleEvent(EVENT_SUMMON_BLAZING_SKELETON, 5000);
                    events.ScheduleEvent(EVENT_HASTEN_SUMMON_TIMER, 30000);
                    events.ScheduleEvent(EVENT_CHECK_EVADE, 10000);
                    events.ScheduleEvent(EVENT_DREAM_PORTAL, 45000);

                    combat_trigger = me->SummonCreature(NPC_GREEN_DRAGON_COMBAT_TRIGGER, me->GetPositionX(), me->GetPositionY(),me->GetPositionZ(), 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 20000);
                    combat_trigger->SetDisplayId(MODEL_INVISIBLE);
                    me->AddThreat(combat_trigger, 10000000.0f);
                    combat_trigger->AddThreat(me, 10000000.0f);
                    me->AI()->AttackStart(combat_trigger);
                    combat_trigger->AI()->AttackStart(me);

                    instance->SetBossState(DATA_VALITHRIA_DREAMWALKER, IN_PROGRESS);

                    //ScriptedAI::MoveInLineOfSight(who);
                }
            }

            void EnterCombat(Unit* /*pWho*/) override
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            }

            void JustSummoned(Creature* summon) override
            {
                summons.Summon(summon);
                if (!summon)
                    return;

                if (!summon->HasAura(SPELL_DREAM_PORTAL_VISUAL_PRE))
                    summon->AI()->AttackStart(me);

                if (summon->GetEntry() == NPC_DREAM_PORTAL_PRE_EFFECT)
                {
                    summon->m_Events.AddEvent(new DelayedCastEvent(summon, SPELL_SUMMON_DREAM_PORTAL, me->GetGUID(), 6000), summon->m_Events.CalculateTime(15000));
                    summon->m_Events.AddEvent(new AuraRemoveEvent(summon, SPELL_DREAM_PORTAL_VISUAL_PRE), summon->m_Events.CalculateTime(15000));
                }
                else if (summon->GetEntry() == NPC_NIGHTMARE_PORTAL_PRE_EFFECT)
                {
                    summon->m_Events.AddEvent(new DelayedCastEvent(summon, SPELL_SUMMON_NIGHTMARE_PORTAL, me->GetGUID(), 6000), summon->m_Events.CalculateTime(15000));
                    summon->m_Events.AddEvent(new AuraRemoveEvent(summon, SPELL_NIGHTMARE_PORTAL_VISUAL_PRE), summon->m_Events.CalculateTime(15000));
                }
            }

            void SummonedCreatureDespawn(Creature* summon) override
            {
                if (summon->GetEntry() == NPC_DREAM_PORTAL || summon->GetEntry() == NPC_NIGHTMARE_PORTAL)
                    if (summon->AI()->GetData(MISSED_PORTALS))
                        ++_missedPortals;
            }

            void JustDied(Unit* /*pKiller*/) override
            {
                Talk(SAY_VALITHRIA_DEATH);

                summons.DespawnAll();
                if(IsHeroic())
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_TWISTED_NIGHTMARE);
                else
                    instance->DoRemoveAurasDueToSpellOnPlayers(EMERALD_VIGOR);

                if (instance)
                    instance->SetBossState(DATA_VALITHRIA_DREAMWALKER, FAIL);
            }

            void SummonCreature(uint32 entry, uint8 probability)
            {
                bool bSuccessfully = false;
                for (int i = 0; i < RAID_MODE(2,4,2,4); ++i)
                    if (urand(0, 100) < probability)
                    {
                        bSuccessfully = true;
                        DoSummon(entry, Pos[i]);
                    }
                //Nobody was summoned - summon at least one
                if (!bSuccessfully)
                    DoSummon(entry, Pos[urand(0, RAID_MODE(1,3,1,3))]);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!instance)
                    return;

                if (instance->GetBossState(DATA_VALITHRIA_DREAMWALKER) == IN_PROGRESS)
                {
                
                    events.Update(diff);
                    while (uint32 eventId = events.ExecuteEvent())
                    {
                        switch (eventId)
                        {
                            case EVENT_SUMMON_ZOMBIE:
                            {
                                SummonCreature(NPC_BLISTERING_ZOMBIE, 70);
                                events.ScheduleEvent(EVENT_SUMMON_ZOMBIE, 30000);
                                break;
                            }
                            case EVENT_SUMMON_ARCHMAGE:
                            {
                                SummonCreature(NPC_RISEN_ARCHMAGE, 50);
                                events.ScheduleEvent(EVENT_SUMMON_ARCHMAGE, 60000);
                                break;
                            }
                            case EVENT_SUMMON_ABOMINATION:
                            {
                                SummonCreature(NPC_GLUTTONOUS_ABOMINATION, 20);
                                events.ScheduleEvent(EVENT_SUMMON_ABOMINATION, 60000);
                                break;
                            }
                            case EVENT_SUMMON_SUPPRESSOR:
                            {
                                SummonCreature(NPC_SUPPRESSER, 40);
                                events.ScheduleEvent(EVENT_SUMMON_SUPPRESSOR, m_uiSummonSuppressorTimer);
                                break;
                            }
                            case EVENT_SUMMON_BLAZING_SKELETON:
                            {
                                SummonCreature(NPC_BLAZING_SKELETON, 40);
                                events.ScheduleEvent(EVENT_SUMMON_BLAZING_SKELETON, m_uiSummonSkeletonTimer);
                                break;
                            }
                            case EVENT_CHECK_EVADE:
                            {
                                events.ScheduleEvent(EVENT_CHECK_EVADE, 10000);
                                Map::PlayerList const &PlList = me->GetMap()->GetPlayers();
                                if (PlList.isEmpty())
                                {
                                    EnterEvadeMode();
                                    return;
                                }

                                for (Map::PlayerList::const_iterator i = PlList.begin(); i != PlList.end(); ++i)
                                    if (Player* pPlayer = i->getSource())
                                        if(pPlayer->isAlive() && pPlayer->IsWithinDistInMap(me, 60.0f,true))
                                            return;

                                EnterEvadeMode();
                                break;
                            }
                            case EVENT_HASTEN_SUMMON_TIMER:
                            {
                                //After 7/? (10/25 player) minutes, Suppressors and Blazing Skeletons start to spawn continuously
                                //(which usually leads to a quick wipe)
                                if (m_uiSummonSuppressorTimer >= 10000)
                                    m_uiSummonSuppressorTimer -= 5000;
                                if (m_uiSummonSkeletonTimer >= 10000)
                                    m_uiSummonSkeletonTimer -= 5000;

                                events.ScheduleEvent(EVENT_HASTEN_SUMMON_TIMER, 30000);
                                break;
                            }
                            case EVENT_DREAM_PORTAL:
                            {
                                if (!IsHeroic())
                                    Talk(SAY_VALITHRIA_DREAM_PORTAL);

                                for (uint32 i = 0; i < _portalCount; ++i)
                                {
                                    DoCast(me, SUMMON_PORTAL);
                                    _openPortals++;
                                }
                               // m_uiPortalTimer = 5000;
                                events.ScheduleEvent(EVENT_DREAM_PORTAL, 45000);
                                break;
                            }
                            default: 
                                break;
                        }
                    }

                  /*  if (m_uiPortalTimer)
                    {
                        if (m_uiPortalTimer <= diff)
                        {
                            if (Is25ManRaid())
                            {
                                for(uint8 p = 0; p < 7; ++p)
                                    DoCast(RAID_MODE(SPELL_PORTAL_N_PRE,SPELL_PORTAL_N_PRE,SPELL_PORTAL_H_PRE,SPELL_PORTAL_H_PRE));
                            }
                            else if (!Is25ManRaid())
                            {
                                for(uint8 p = 0; p < 3; ++p)
                                    DoCast(RAID_MODE(SPELL_PORTAL_N_PRE,SPELL_PORTAL_N_PRE,SPELL_PORTAL_H_PRE,SPELL_PORTAL_H_PRE));
                            }
                        m_uiPortalTimer = 0;

                        }
                        else m_uiPortalTimer -= diff;
                    }*/

                    if (!bAboveHP && (HealthAbovePct(74)))
                    {
                        Talk(SAY_VALITHRIA_75_PERCENT);
                        bAboveHP = true;
                    }

                    if (!bBelowHP && (HealthBelowPct(26)))
                    {
                        Talk(SAY_VALITHRIA_25_PERCENT);
                        bBelowHP = true;
                    }

                    if (HealthBelowPct(3))
                    {
                        if (instance)
                            instance->SetBossState(DATA_VALITHRIA_DREAMWALKER, FAIL);
                        summons.DespawnAll();
                        Reset();
                        EnterEvadeMode();
                    }

                    if ((HealthAbovePct(99)) && !bEnd)
                    {
                        events.CancelEvent(EVENT_SUMMON_ZOMBIE);
                        events.CancelEvent(EVENT_SUMMON_ARCHMAGE);
                        events.CancelEvent(EVENT_SUMMON_ABOMINATION);
                        events.CancelEvent(EVENT_SUMMON_SUPPRESSOR);
                        events.CancelEvent(EVENT_SUMMON_BLAZING_SKELETON);
                        events.CancelEvent(EVENT_HASTEN_SUMMON_TIMER);
                        events.CancelEvent(EVENT_CHECK_EVADE);
                        events.CancelEvent(EVENT_DREAM_PORTAL);

                        Talk(SAY_VALITHRIA_SUCCESS);

                        if (_openPortals == _missedPortals)
                            instance->DoUpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET, SPELL_ACHIEVEMENT_CHECK, 0, 0, me);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                        me->SetReactState(REACT_PASSIVE);
                        me->RemoveAurasDueToSpell(SPELL_CORRUPTION);

                        bEnd = true;
                    }

                    if (bEnd)
                    {
                        if (m_uiEndTimer <= diff)
                        {
                            switch(m_uiStage)
                            {
                                case 1:
                                    Talk(SAY_VALITHRIA_BERSERK);
                                    DoCastAOE(SPELL_RAGE);
                                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                                    ++m_uiStage;
                                    m_uiEndTimer = 6000;
                                    break;
                                case 2:
                                    {
                                        //if (combat_trigger && combat_trigger->isAlive())
                                        //    combat_trigger->DespawnOrUnsummon();
                                        DoCast(me, SPELL_DREAM_SLIP);
                                        ++m_uiStage;
                                        m_uiEndTimer = 1000;
                                    }
                                    break;
                                case 3:
                                    instance->SetBossState(DATA_VALITHRIA_DREAMWALKER, DONE);
                                    DoCast(me, SPELL_CANCEL_ALL_AURAS, true);
                                    DoCast(me, SPELL_AWARD_REPUTATION_BOSS_KILL, true);
                                    summons.DespawnAll();
                                    // this display id was found in sniff instead of the one on aura
                                    me->SetDisplayId(11686);
                                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                                    me->DespawnOrUnsummon(4000);
                                    m_uiEndTimer = 1000;
                                    bEnd = false;
                                    despawn = true;
                                    ++m_uiStage;
                                    break;
                            }
                        } else m_uiEndTimer -= diff;
                    }
                }
            }

           

        private:
            InstanceScript* instance;
            Creature* combat_trigger;

            uint32 m_uiSummonSkeletonTimer, m_uiSummonSuppressorTimer;
            uint8 m_uiStage;
            uint32 m_uiPortalTimer;
            uint32 m_uiEndTimer;
            uint32 m_uiSummonTimer;
            uint32 const _portalCount;
            uint8 _missedPortals;
            uint8 _openPortals;
            bool bIntro;
            bool bEnd;
            bool bAboveHP;
            bool bBelowHP;
            bool despawn;
            SummonList summons;
        };

        CreatureAI* GetAI(Creature* pCreature) const override
        {
            return new boss_valithria_dreamwalkerAI(pCreature);
        }
};

class npc_risen_archmage : public CreatureScript
{
    public:
        npc_risen_archmage() : CreatureScript("npc_risen_archmage") { }

        struct npc_risen_archmageAI : public ScriptedAI
        {
            npc_risen_archmageAI(Creature* pCreature) : ScriptedAI(pCreature)
            {
                instance = pCreature->GetInstanceScript();
            }

            void Reset() override
            {
                m_uiVolleyTimer = 12000;
                m_uiColumnTimer = 20000;
                m_uiVoidTimer = 30000;
            }

            void EnterCombat(Unit* /*who*/) override
            {
            }

            void KilledUnit(Unit* /*victim*/) override
            {
                //DoScriptText(SAY_VALITHRIA_PLAYER_DEATH, pValithria);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (m_uiVolleyTimer <= diff)
                {
                    if(Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 1))
                        DoCast(pTarget, SPELL_VOLLEY);
                    m_uiVolleyTimer = 15000;
                } else m_uiVolleyTimer -= diff;

                if (m_uiVoidTimer <= diff)
                {
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, ManaVoidSelector(me)))
                        DoCast(target, SPELL_MANA_VOID);
                    m_uiVoidTimer = 30000;
                } else m_uiVoidTimer -= diff;

                if (m_uiColumnTimer <= diff)
                {
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, -10.0f, true))
                        DoCast(target, SPELL_COLUMN_OF_FROST);
                    m_uiColumnTimer = 20000;
                } else m_uiColumnTimer -= diff;

                DoMeleeAttackIfReady();
            }
        private:
            InstanceScript* instance;

            uint32 m_uiVolleyTimer;
            uint32 m_uiColumnTimer;
            uint32 m_uiVoidTimer;
        };

        CreatureAI* GetAI(Creature* pCreature) const override
        {
            return new npc_risen_archmageAI(pCreature);
        }
};

class npc_blazing_skeleton : public CreatureScript
{
    public:
        npc_blazing_skeleton() : CreatureScript("npc_blazing_skeleton") { }

        struct npc_blazing_skeletonAI : public ScriptedAI
        {
            npc_blazing_skeletonAI(Creature* pCreature) : ScriptedAI(pCreature) { }

            void Reset() override
            {
                m_uiWasteTimer = 20000;
                m_uiFireballTimer = 5000;
            }

            void EnterCombat(Unit* /*who*/) override { }

            void KilledUnit(Unit* /*pVictim*/) override
            {
                //DoScriptText(SAY_VALITHRIA_PLAYER_DEATH, pValithria);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                if (m_uiWasteTimer <= diff)
                {
                    DoCast(SPELL_WASTE);
                    m_uiWasteTimer = 20000;
                } else m_uiWasteTimer -= diff;

                if (m_uiFireballTimer <= diff)
                {
                    if(Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 1))
                        DoCast(pTarget, SPELL_FIREBALL);
                    m_uiFireballTimer = 5000;
                } else m_uiFireballTimer -= diff;

                DoMeleeAttackIfReady();
            }
        private:
            uint32 m_uiWasteTimer;
            uint32 m_uiFireballTimer;
        };

        CreatureAI* GetAI(Creature* pCreature) const override
        {
            return new npc_blazing_skeletonAI(pCreature);
        }
};

class npc_suppresser : public CreatureScript
{
    public:
        npc_suppresser() : CreatureScript("npc_suppresser") { }

        struct npc_suppresserAI : public ScriptedAI
        {
            npc_suppresserAI(Creature* pCreature) : ScriptedAI(pCreature)
            {
                instance = pCreature->GetInstanceScript();
            }

            void EnterCombat(Unit* /*who*/) override
            {
                me->SetReactState(REACT_PASSIVE);
                m_uiCheckTimer = 2500;
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                if (m_uiCheckTimer <= diff)
                {
                    if (Creature* pValithria = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_VALITHRIA_DREAMWALKER)))
                        me->CastSpell(pValithria, SPELL_SUPRESSION, true);
                    m_uiCheckTimer = 100000;
                } else m_uiCheckTimer -= diff;
            }

        private:
            uint32 m_uiCheckTimer;
            InstanceScript* instance;
        };

        CreatureAI* GetAI(Creature* pCreature) const override
        {
            return new npc_suppresserAI(pCreature);
        }
};

class npc_gluttonous_abomination : public CreatureScript
{
    public:
        npc_gluttonous_abomination() : CreatureScript("npc_gluttonous_abomination") { }

        struct npc_gluttonous_abominationAI : public ScriptedAI
        {
            npc_gluttonous_abominationAI(Creature* pCreature) : ScriptedAI(pCreature)
            {
                instance = pCreature->GetInstanceScript();
            }

            void EnterCombat(Unit* /*who*/) override { }

            void Reset() override
            {
                m_uiSprayTimer = 10000;
            }

            void KilledUnit(Unit* /*pVictim*/) override
            {
                //DoScriptText(SAY_VALITHRIA_PLAYER_DEATH, pValithria);
            }

            void JustDied(Unit* /*killer*/) override
            {
                Creature* pValithria = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_VALITHRIA_DREAMWALKER));
                for (uint8 i = 1; i < 5; i++)
                {
                    if(pValithria)
                        pValithria->SummonCreature(NPC_ROT_WORM, me->GetPositionX()+urand(3,6), me->GetPositionY()+urand(3,6), me->GetPositionZ(), 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
                }
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                if (m_uiSprayTimer <= diff)
                {
                    DoCast(me, SPELL_SPRAY);
                    m_uiSprayTimer = 20000;
                } else m_uiSprayTimer -= diff;

                DoMeleeAttackIfReady();
            }
        private:
            InstanceScript* instance;
            uint32 m_uiSprayTimer;
        };

        CreatureAI* GetAI(Creature* pCreature) const override
        {
            return new npc_gluttonous_abominationAI(pCreature);
        }
};

class npc_blistering_zombie : public CreatureScript
{
    public:
        npc_blistering_zombie() : CreatureScript("npc_blistering_zombie") { }

        struct npc_blistering_zombieAI : public ScriptedAI
        {
            npc_blistering_zombieAI(Creature* pCreature) : ScriptedAI(pCreature) { }

            void EnterCombat(Unit* /*who*/) override { }

            void Reset() override
            {
                m_uiBurstTimer = 20000;
                m_uiDelayTimer = 99999;
            }

            void KilledUnit(Unit* /*victim*/) override
            {
                //DoScriptText(SAY_VALITHRIA_PLAYER_DEATH, pValithria);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                if (m_uiBurstTimer <= diff)
                {
                    DoCast(SPELL_BURST);
                    m_uiBurstTimer = 20000;
                    m_uiDelayTimer = 1000;
                } else m_uiBurstTimer -= diff;

                if (m_uiDelayTimer <= diff)
                {
                    me->DespawnOrUnsummon();
                    m_uiDelayTimer = 100000;
                } else m_uiDelayTimer -= diff;

                DoMeleeAttackIfReady();
            }
        private:
            uint32 m_uiBurstTimer;
            uint32 m_uiDelayTimer;
        };

        CreatureAI* GetAI(Creature* pCreature) const override
        {
            return new npc_blistering_zombieAI(pCreature);
        }
};

class npc_dream_portal : public CreatureScript
{
    public:
        npc_dream_portal() : CreatureScript("npc_dream_portal") { }

        struct npc_dream_portalAI : public CreatureAI
        {
            npc_dream_portalAI(Creature* creature) : CreatureAI(creature),
                _used(false)
            {
            }

            void OnSpellClick(Unit* /*clicker*/) override
            {
                _used = true;
                me->DespawnOrUnsummon();
            }

            uint32 GetData(uint32 type) const override
            {
                return (type == MISSED_PORTALS && _used) ? 1 : 0;
            }

            void UpdateAI(uint32 /*diff*/) override
            {
                UpdateVictim();
            }

        private:
            bool _used;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetIcecrownCitadelAI<npc_dream_portalAI>(creature);
        }
};

class npc_dream_cloud : public CreatureScript
{
    public:
        npc_dream_cloud() : CreatureScript("npc_dream_cloud") { }

        struct npc_dream_cloudAI : public ScriptedAI
        {
            npc_dream_cloudAI(Creature* creature) : ScriptedAI(creature),
                _instance(creature->GetInstanceScript())
            {
            }

            void Reset() override
            {
                _events.Reset();
                _events.ScheduleEvent(EVENT_CHECK_PLAYER, 1000);
                me->SetCorpseDelay(0);  // remove corpse immediately
                me->LoadCreaturesAddon(true);
            }

            void UpdateAI(uint32 diff) override
            {
                // trigger
                if (_instance->GetBossState(DATA_VALITHRIA_DREAMWALKER) != IN_PROGRESS)
                    return;

                _events.Update(diff);

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CHECK_PLAYER:
                        {
                            Player* player = NULL;
                            Trinity::AnyPlayerInObjectRangeCheck check(me, 5.0f);
                            Trinity::PlayerSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(me, player, check);
                            Trinity::VisitNearbyWorldObject(me, 7.5f, searcher);
                            _events.ScheduleEvent(player ? EVENT_EXPLODE : EVENT_CHECK_PLAYER, 1000);
                            break;
                        }
                        case EVENT_EXPLODE:
                            me->GetMotionMaster()->MoveIdle();
                            // must use originalCaster the same for all clouds to allow stacking
                            me->CastSpell(me, EMERALD_VIGOR, false, NULL, NULL, _instance->GetGuidData(DATA_VALITHRIA_DREAMWALKER));
                            me->DespawnOrUnsummon(100);
                            break;
                        default:
                            break;
                    }
                }
            }

        private:
            EventMap _events;
            InstanceScript* _instance;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetIcecrownCitadelAI<npc_dream_cloudAI>(creature);
        }
};

class spell_dreamwalker_mana_void : public SpellScriptLoader
{
    public:
        spell_dreamwalker_mana_void() : SpellScriptLoader("spell_dreamwalker_mana_void") { }

        class spell_dreamwalker_mana_void_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dreamwalker_mana_void_AuraScript);

            void PeriodicTick(AuraEffect const* aurEff)
            {
                // first 3 ticks have amplitude 1 second
                // remaining tick every 500ms
                if (aurEff->GetTickNumber() <= 5)
                    if (!(aurEff->GetTickNumber() & 1))
                        PreventDefaultAction();
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_dreamwalker_mana_void_AuraScript::PeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dreamwalker_mana_void_AuraScript();
        }
};

class spell_dreamwalker_decay_periodic_timer : public SpellScriptLoader
{
    public:
        spell_dreamwalker_decay_periodic_timer() : SpellScriptLoader("spell_dreamwalker_decay_periodic_timer") { }

        class spell_dreamwalker_decay_periodic_timer_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dreamwalker_decay_periodic_timer_AuraScript);

            bool Load() override
            {
                _decayRate = GetId() != SPELL_TIMER_BLAZING_SKELETON ? 1000 : 5000;
                return true;
            }

            void DecayPeriodicTimer(AuraEffect* aurEff)
            {
                int32 timer = aurEff->GetPeriodicTimer();
                if (timer <= 5)
                    return;

                aurEff->SetPeriodicTimer(timer - _decayRate);
            }

            void Register() override
            {
                OnEffectUpdatePeriodic += AuraEffectUpdatePeriodicFn(spell_dreamwalker_decay_periodic_timer_AuraScript::DecayPeriodicTimer, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }

            int32 _decayRate;
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dreamwalker_decay_periodic_timer_AuraScript();
        }
};

class spell_dreamwalker_summoner : public SpellScriptLoader
{
    public:
        spell_dreamwalker_summoner() : SpellScriptLoader("spell_dreamwalker_summoner") { }

        class spell_dreamwalker_summoner_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dreamwalker_summoner_SpellScript);

            bool Load() override
            {
                if (!GetCaster()->GetInstanceScript())
                    return false;
                return true;
            }

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                targets.remove_if(Trinity::UnitAuraCheck(true, SPELL_RECENTLY_SPAWNED));
                if (targets.empty())
                    return;

                WorldObject* target = Trinity::Containers::SelectRandomContainerElement(targets);
                targets.clear();
                targets.push_back(target);
            }

            void HandleForceCast(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                if (!GetHitUnit())
                    return;

                GetHitUnit()->CastSpell(GetCaster(), GetSpellInfo()->Effects[effIndex]->TriggerSpell, true, NULL, NULL, GetCaster()->GetInstanceScript()->GetGuidData(DATA_VALITHRIA_LICH_KING));
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dreamwalker_summoner_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
                OnEffectHitTarget += SpellEffectFn(spell_dreamwalker_summoner_SpellScript::HandleForceCast, EFFECT_0, SPELL_EFFECT_FORCE_CAST);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dreamwalker_summoner_SpellScript();
        }
};

class spell_dreamwalker_summon_suppresser : public SpellScriptLoader
{
    public:
        spell_dreamwalker_summon_suppresser() : SpellScriptLoader("spell_dreamwalker_summon_suppresser") { }

        class spell_dreamwalker_summon_suppresser_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dreamwalker_summon_suppresser_AuraScript);

            void PeriodicTick(AuraEffect const* /*aurEff*/)
            {
                PreventDefaultAction();
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                std::list<Creature*> summoners;
                GetCreatureListWithEntryInGrid(summoners, caster, NPC_WORLD_TRIGGER, 100.0f);
                summoners.remove_if(Trinity::UnitAuraCheck(true, SPELL_RECENTLY_SPAWNED));
                Trinity::Containers::RandomResizeList(summoners, 2);
                if (summoners.empty())
                    return;

                for (uint32 i = 0; i < 3; ++i)
                    caster->CastSpell(summoners.front(), SPELL_SUMMON_SUPPRESSER, true);
                for (uint32 i = 0; i < 3; ++i)
                    caster->CastSpell(summoners.back(), SPELL_SUMMON_SUPPRESSER, true);
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_dreamwalker_summon_suppresser_AuraScript::PeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dreamwalker_summon_suppresser_AuraScript();
        }
};

class spell_dreamwalker_summon_suppresser_effect : public SpellScriptLoader
{
    public:
        spell_dreamwalker_summon_suppresser_effect() : SpellScriptLoader("spell_dreamwalker_summon_suppresser_effect") { }

        class spell_dreamwalker_summon_suppresser_effect_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dreamwalker_summon_suppresser_effect_SpellScript);

            bool Load() override
            {
                if (!GetCaster()->GetInstanceScript())
                    return false;
                return true;
            }

            void HandleForceCast(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                if (!GetHitUnit())
                    return;

                GetHitUnit()->CastSpell(GetCaster(), GetSpellInfo()->Effects[effIndex]->TriggerSpell, true, NULL, NULL, GetCaster()->GetInstanceScript()->GetGuidData(DATA_VALITHRIA_LICH_KING));
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_dreamwalker_summon_suppresser_effect_SpellScript::HandleForceCast, EFFECT_0, SPELL_EFFECT_FORCE_CAST);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dreamwalker_summon_suppresser_effect_SpellScript();
        }
};

class spell_dreamwalker_summon_dream_portal : public SpellScriptLoader
{
    public:
        spell_dreamwalker_summon_dream_portal() : SpellScriptLoader("spell_dreamwalker_summon_dream_portal") { }

        class spell_dreamwalker_summon_dream_portal_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dreamwalker_summon_dream_portal_SpellScript);

            void HandleScript(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                if (!GetHitUnit())
                    return;

                uint32 spellId = RAND(71301, 72220, 72223, 72225);
                GetHitUnit()->CastSpell(GetHitUnit(), spellId, true);
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_dreamwalker_summon_dream_portal_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dreamwalker_summon_dream_portal_SpellScript();
        }
};

class spell_dreamwalker_summon_nightmare_portal : public SpellScriptLoader
{
    public:
        spell_dreamwalker_summon_nightmare_portal() : SpellScriptLoader("spell_dreamwalker_summon_nightmare_portal") { }

        class spell_dreamwalker_summon_nightmare_portal_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dreamwalker_summon_nightmare_portal_SpellScript);

            void HandleScript(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                if (!GetHitUnit())
                    return;

                uint32 spellId = RAND(71977, 72481, 72482, 72483);
                GetHitUnit()->CastSpell(GetHitUnit(), spellId, true);
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_dreamwalker_summon_nightmare_portal_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dreamwalker_summon_nightmare_portal_SpellScript();
        }
};

class spell_dreamwalker_nightmare_cloud : public SpellScriptLoader
{
    public:
        spell_dreamwalker_nightmare_cloud() : SpellScriptLoader("spell_dreamwalker_nightmare_cloud") { }

        class spell_dreamwalker_nightmare_cloud_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dreamwalker_nightmare_cloud_AuraScript);

            bool Load() override
            {
                _instance = GetOwner()->GetInstanceScript();
                return _instance != NULL;
            }

            void PeriodicTick(AuraEffect const* /*aurEff*/)
            {
                if (_instance->GetBossState(DATA_VALITHRIA_DREAMWALKER) != IN_PROGRESS)
                    PreventDefaultAction();
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_dreamwalker_nightmare_cloud_AuraScript::PeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }

            InstanceScript* _instance;
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dreamwalker_nightmare_cloud_AuraScript();
        }
};

class spell_dreamwalker_twisted_nightmares : public SpellScriptLoader
{
    public:
        spell_dreamwalker_twisted_nightmares() : SpellScriptLoader("spell_dreamwalker_twisted_nightmares") { }

        class spell_dreamwalker_twisted_nightmares_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dreamwalker_twisted_nightmares_SpellScript);

            void HandleScript(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                // impossible with TARGET_UNIT_CASTER
                //if (!GetHitUnit())
                //    return;

                if (InstanceScript* instance = GetHitUnit()->GetInstanceScript())
                    GetHitUnit()->CastSpell((Unit*)NULL, GetSpellInfo()->Effects[effIndex]->TriggerSpell, true, NULL, NULL, instance->GetGuidData(DATA_VALITHRIA_DREAMWALKER));
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_dreamwalker_twisted_nightmares_SpellScript::HandleScript, EFFECT_2, SPELL_EFFECT_FORCE_CAST);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dreamwalker_twisted_nightmares_SpellScript();
        }
};

void AddSC_boss_valithria_dreamwalker()
{
    new boss_valithria_dreamwalker();
    new npc_risen_archmage();
    new npc_blazing_skeleton();
    new npc_dream_portal();
    new npc_suppresser();
    new npc_gluttonous_abomination();
    new npc_blistering_zombie();
    new npc_dream_cloud();
    new spell_dreamwalker_mana_void();
    new spell_dreamwalker_decay_periodic_timer();
    new spell_dreamwalker_summoner();
    new spell_dreamwalker_summon_suppresser();
    new spell_dreamwalker_summon_suppresser_effect();
    new spell_dreamwalker_summon_dream_portal();
    new spell_dreamwalker_summon_nightmare_portal();
    new spell_dreamwalker_nightmare_cloud();
    new spell_dreamwalker_twisted_nightmares();
    //new achievement_portal_jockey();  
}