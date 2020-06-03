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

#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "PassiveAI.h"
#include "GameObjectAI.h"
#include "MapManager.h"
#include "MoveSplineInit.h"
#include "ulduar.h"

enum Texts
{
    SAY_BRANN_ALGALON_INTRO_1       = 0,
    SAY_BRANN_ALGALON_INTRO_2       = 1,
    SAY_BRANN_ALGALON_OUTRO         = 2,

    SAY_ALGALON_INTRO_1             = 0,
    SAY_ALGALON_INTRO_2             = 1,
    SAY_ALGALON_INTRO_3             = 2,
    SAY_ALGALON_START_TIMER         = 3,
    SAY_ALGALON_AGGRO               = 4,
    SAY_ALGALON_COLLAPSING_STAR     = 5,
    EMOTE_ALGALON_COLLAPSING_STAR   = 6,
    SAY_ALGALON_BIG_BANG            = 7,
    EMOTE_ALGALON_BIG_BANG          = 8,
    SAY_ALGALON_ASCEND              = 9,
    EMOTE_ALGALON_COSMIC_SMASH      = 10,
    SAY_ALGALON_PHASE_TWO           = 11,
    SAY_ALGALON_OUTRO_1             = 12,
    SAY_ALGALON_OUTRO_2             = 13,
    SAY_ALGALON_OUTRO_3             = 14,
    SAY_ALGALON_OUTRO_4             = 15,
    SAY_ALGALON_OUTRO_5             = 16,
    SAY_ALGALON_DESPAWN_1           = 17,
    SAY_ALGALON_DESPAWN_2           = 18,
    SAY_ALGALON_DESPAWN_3           = 19,
    SAY_ALGALON_KILL                = 20,
};

enum Spells
{
    // Algalon the Observer
    SPELL_ARRIVAL                       = 64997,
    SPELL_RIDE_THE_LIGHTNING            = 64986,
    SPELL_SUMMON_AZEROTH                = 64994,
    SPELL_REORIGINATION                 = 64996,
    SPELL_SUPERMASSIVE_FAIL             = 65311,
    SPELL_QUANTUM_STRIKE                = 64395,
    SPELL_PHASE_PUNCH                   = 64412,
    SPELL_BIG_BANG                      = 64443,
    SPELL_ASCEND_TO_THE_HEAVENS         = 64487,
    SPELL_COSMIC_SMASH                  = 62301,
    SPELL_COSMIC_SMASH_TRIGGERED        = 62304,
    SPELL_COSMIC_SMASH_VISUAL_STATE     = 62300,
    SPELL_SELF_STUN                     = 65256,
    SPELL_KILL_CREDIT                   = 65184,
    SPELL_TELEPORT                      = 62940,

    // Algalon Stalker
    SPELL_TRIGGER_3_ADDS                = 62266,    // Triggers Living Constellation

    // Living Constellation
    SPELL_ARCANE_BARRAGE                = 64599,

    // Collapsing Star
    SPELL_COLLAPSE                      = 62018,
    SPELL_BLACK_HOLE_SPAWN_VISUAL       = 62003,
    SPELL_SUMMON_BLACK_HOLE             = 62189,

    // Black Hole
    SPELL_BLACK_HOLE_TRIGGER            = 62185,
    SPELL_CONSTELLATION_PHASE_TRIGGER   = 65508,
    SPELL_CONSTELLATION_PHASE_EFFECT    = 65509,
    SPELL_BLACK_HOLE_EXPLOSION          = 64122,
    SPELL_SUMMON_VOID_ZONE_VISUAL       = 64470,
    SPELL_VOID_ZONE_VISUAL              = 64469,
    SPELL_BLACK_HOLE_CREDIT             = 65312,

    // Worm Hole
    SPELL_WORM_HOLE_TRIGGER             = 65251,
    SPELL_SUMMON_UNLEASHED_DARK_MATTER  = 64450,
};

uint32 const PhasePunchAlphaId[5] = {64435, 64434, 64428, 64421, 64417};

enum Events
{
    // Celestial Planetarium Access
    EVENT_DESPAWN_CONSOLE           = 1,

    // Brann Bronzebeard
    EVENT_BRANN_MOVE_INTRO          = 2,
    EVENT_SUMMON_ALGALON            = 3,
    EVENT_BRANN_OUTRO_1             = 4,
    EVENT_BRANN_OUTRO_2             = 5,

    // Algalon the Observer
    EVENT_INTRO_1                   = 6,
    EVENT_INTRO_2                   = 7,
    EVENT_INTRO_3                   = 8,
    EVENT_INTRO_FINISH              = 9,
    EVENT_START_COMBAT              = 10,
    EVENT_INTRO_TIMER_DONE          = 11,
    EVENT_QUANTUM_STRIKE            = 12,
    EVENT_PHASE_PUNCH               = 13,
    EVENT_SUMMON_COLLAPSING_STAR    = 14,
    EVENT_BIG_BANG                  = 15,
    EVENT_RESUME_UPDATING           = 16,
    EVENT_ASCEND_TO_THE_HEAVENS     = 17,
    EVENT_EVADE                     = 18,
    EVENT_COSMIC_SMASH              = 19,
    EVENT_UNLOCK_YELL               = 20,
    EVENT_OUTRO_START               = 21,
    EVENT_OUTRO_1                   = 22,
    EVENT_OUTRO_2                   = 23,
    EVENT_OUTRO_3                   = 24,
    EVENT_OUTRO_4                   = 25,
    EVENT_OUTRO_5                   = 26,
    EVENT_OUTRO_6                   = 27,
    EVENT_OUTRO_7                   = 28,
    EVENT_OUTRO_8                   = 29,
    EVENT_OUTRO_9                   = 30,
    EVENT_OUTRO_10                  = 31,
    EVENT_OUTRO_11                  = 32,
    EVENT_OUTRO_12                  = 33,
    EVENT_OUTRO_13                  = 34,
    EVENT_OUTRO_14                  = 35,
    EVENT_DESPAWN_ALGALON_1         = 36,
    EVENT_DESPAWN_ALGALON_2         = 37,
    EVENT_DESPAWN_ALGALON_3         = 38,

    // Living Constellation
    EVENT_ARCANE_BARRAGE            = 39,
    EVENT_LIVING_CONSTELLATION      = 40,
    
    EVENT_CHECK_EQUIP               = 41,
};

enum Actions
{
    ACTION_START_INTRO      = 0,
    ACTION_FINISH_INTRO     = 1,
    ACTION_ACTIVATE_STAR    = 2,
    ACTION_BIG_BANG         = 3,
    ACTION_ASCEND           = 4,
    ACTION_OUTRO            = 5,
    ACTION_BLACK_HOLE_VAL   = 7,
};

enum Points
{
    POINT_BRANN_INTRO           = 0,
    MAX_BRANN_WAYPOINTS_INTRO   = 10,
    POINT_BRANN_OUTRO           = 10,
    POINT_BRANN_OUTRO_END       = 11,

    POINT_ALGALON_LAND          = 1,
    POINT_ALGALON_OUTRO         = 2,
};

enum Phases
{
    PHASE_NULL,            
    PHASE_ONE,             
    PHASE_TWO,             
    PHASE_WIN,            

};

enum AchievmentInfo
{
    EVENT_ID_SUPERMASSIVE_START = 21697,
    DATA_HAS_FED_ON_TEARS       = 30043005,
};


Position const BrannIntroSpawnPos = {1676.277f, -162.5308f, 427.3326f, 3.235537f};
Position const BrannIntroWaypoint[MAX_BRANN_WAYPOINTS_INTRO] =
{
    {1642.482f, -164.0812f, 427.2602f, 0.0f},
    {1635.000f, -169.5145f, 427.2523f, 0.0f},
    {1632.814f, -173.9334f, 427.2621f, 0.0f},
    {1632.676f, -190.5927f, 425.8831f, 0.0f},
    {1631.497f, -214.2221f, 418.1152f, 0.0f},
    {1624.717f, -224.6876f, 418.1152f, 0.0f},
    {1631.497f, -214.2221f, 418.1152f, 0.0f},
    {1632.676f, -190.5927f, 425.8831f, 0.0f},
    {1632.814f, -173.9334f, 427.2621f, 0.0f},
    {1635.000f, -169.5145f, 427.2523f, 0.0f},
};
Position const AlgalonSummonPos = {1632.531f, -304.8516f, 450.1123f, 1.530165f};
Position const AlgalonLandPos   = {1632.668f, -302.7656f, 417.3211f, 1.530165f};

#define LIVING_CONSTELLATION_COUNT 11
Position const ConstellationPos[6] =
{
    {1631.675f, -274.057f, 440.000f, 0.0f},
    {1631.782f, -331.164f, 440.000f, 0.0f},
    {1655.816f, -322.583f, 440.000f, 0.0f},
    {1605.966f, -318.651f, 440.000f, 0.0f},
    {1657.084f, -291.127f, 440.000f, 0.0f},
    {1608.160f, -292.077f, 440.000f, 0.0f},
};

#define COLLAPSING_STAR_COUNT 4
Position const CollapsingStarPos[COLLAPSING_STAR_COUNT] =
{
    {1649.438f, -319.8127f, 418.3941f, 1.082104f},
    {1647.005f, -288.6790f, 418.3955f, 3.490659f},
    {1622.451f, -321.1563f, 418.3988f, 4.677482f},
    {1615.060f, -291.6816f, 418.3996f, 3.490659f},
};

Position const darkmatterpos[6] =
{
    {1631.675f, -274.057f, 418.000f, 0.0f},
    {1631.782f, -331.164f, 418.000f, 0.0f},
    {1655.816f, -322.583f, 418.000f, 0.0f},
    {1605.966f, -318.651f, 418.000f, 0.0f},
    {1657.084f, -291.127f, 418.000f, 0.0f},
    {1608.160f, -292.077f, 418.000f, 0.0f},
};

Position const blackholepos[4] =
{
    {1612.3981f, -284.5500f, 417.321f, 0.0f},
    {1650.8868f, -286.9484f, 417.321f, 0.0f},
    {1656.0089f, -322.9095f, 417.321f, 0.0f},
    {1611.7705f, -327.7570f, 417.321f, 0.0f},
};

Position const AlgalonOutroPos = {1633.64f, -317.78f, 417.3211f, 0.0f};
Position const BrannOutroPos[3] =
{
    {1632.023f, -243.7434f, 417.9118f, 0.0f},
    {1631.986f, -297.7831f, 417.3210f, 0.0f},
    {1633.832f, -216.2948f, 417.0463f, 0.0f},
};

class CosmicSmashDamageEvent : public BasicEvent
{
    public:
        CosmicSmashDamageEvent(Unit* caster) : _caster(caster)
        {
        }

        bool Execute(uint64 /*execTime*/, uint32 /*diff*/) override
        {
            _caster->CastSpell((Unit*)NULL, SPELL_COSMIC_SMASH_TRIGGERED, TRIGGERED_FULL_MASK);
            return true;
        }

    private:
        Unit* _caster;
};

class boss_algalon_the_observer : public CreatureScript
{
    public:
        boss_algalon_the_observer() : CreatureScript("boss_algalon_the_observer") {}

        struct boss_algalon_the_observerAI : public BossAI
        {
            boss_algalon_the_observerAI(Creature* creature) : BossAI(creature, BOSS_ALGALON), phase(PHASE_NULL)
            {
                me->SetPhaseMask(65519, true);
                _fedOnTears = true;
                instance = creature->GetInstanceScript();
            }
            InstanceScript * instance;

            Phases phase;
            uint32 RemoveFactionAura;
            
            void Reset() override
            {
                _Reset();
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->SetReactState(REACT_PASSIVE);
                _phaseTwo = false;
                _fightWon = false;
                RemoveFactionAura = 0;
                phase = PHASE_NULL;
                DoCast(me, 42459, true);
                SetEquipmentSlots(false, 45607, 45607, EQUIP_NO_CHANGE);
            }

            bool GetPhase()
            {
                return _phaseTwo;
            }

            bool GetfedOnTears()
            {
                return _fedOnTears;
            }

            void KilledUnit(Unit* who) override
            {
                if (who->GetTypeId() == TYPEID_PLAYER)
                {
                    Talk(SAY_ALGALON_KILL);
                    if (_fedOnTears)
                        _fedOnTears = false;
                }
            }

            void DoAction(int32 const action) override
            {
                switch (action)
                {
                    case ACTION_START_INTRO:
                    {
                        me->SetFlag(UNIT_FIELD_FLAGS_2, 0x20);
                        me->SetDisableGravity(true);
                        DoCast(me, SPELL_ARRIVAL, true);
                        DoCast(me, SPELL_RIDE_THE_LIGHTNING, true);
                        me->GetMotionMaster()->MovePoint(POINT_ALGALON_LAND, AlgalonLandPos);
                        me->SetHomePosition(AlgalonLandPos);
                        Movement::MoveSplineInit init(*me);
                        init.MoveTo(AlgalonLandPos.GetPositionX(), AlgalonLandPos.GetPositionY(), AlgalonLandPos.GetPositionZ());
                        init.SetOrientationFixed(true);
                        init.Launch();
                        events.Reset();
                        events.ScheduleEvent(EVENT_INTRO_1, 5000);
                        events.ScheduleEvent(EVENT_INTRO_2, 15000);
                        events.ScheduleEvent(EVENT_INTRO_3, 23000);
                        events.ScheduleEvent(EVENT_INTRO_FINISH, 36000);
                        break;
                    }
                    case ACTION_ASCEND:
                        events.CancelEvent(EVENT_RESUME_UPDATING);
                        events.ScheduleEvent(EVENT_ASCEND_TO_THE_HEAVENS, 1500);
                        break;
                    case EVENT_DESPAWN_ALGALON:
                        events.Reset();
                        if (me->isInCombat())
                            events.ScheduleEvent(EVENT_ASCEND_TO_THE_HEAVENS, 1);
                        events.ScheduleEvent(EVENT_DESPAWN_ALGALON_1, 5000);
                        events.ScheduleEvent(EVENT_DESPAWN_ALGALON_2, 17000);
                        events.ScheduleEvent(EVENT_DESPAWN_ALGALON_3, 26000);
                        me->DespawnOrUnsummon(34000);
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_NPC);
                        break;
                    case ACTION_INIT_ALGALON:
                        _firstPull = false;
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                        break;
                    case ACTION_BLACK_HOLE_VAL:
                        instance->DoStartTimedAchievement(CRITERIA_TIMED_TYPE_EVENT2, EVENT_ID_SUPERMASSIVE_START);
                        instance->DoUpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET, SPELL_BLACK_HOLE_CREDIT);
                        me->MonsterTextEmote("BlackHole Despawn", ObjectGuid::Empty, true);
                        break;
                }
            }
            
            void EnterCombat(Unit* /*target*/) override
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_NPC);
                instance->SetBossState(BOSS_ALGALON, IN_PROGRESS);
                events.Reset();
                phase = PHASE_ONE;

                if (instance)
                {
                    instance->DoRemoveAurasDueToSpellOnPlayers(55773);
                    instance->DoRemoveAurasDueToSpellOnPlayers(55774);
                    RemoveFactionAura = 2000;
                }

                for (uint32 n = 0; n < 6; n++)
                {
                    if (Creature * matter = me->SummonCreature(33089, darkmatterpos[n], TEMPSUMMON_CORPSE_DESPAWN))
                           matter->SetPhaseMask(16, true);
                }
                
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_NPC);
                me->SetReactState(REACT_DEFENSIVE);
                me->SetSheath(SHEATH_STATE_MELEE);
                Talk(SAY_ALGALON_AGGRO);
                _EnterCombat();
                me->setActive(true);
                DoZoneInCombat();
                events.ScheduleEvent(EVENT_QUANTUM_STRIKE, 3500);
                events.ScheduleEvent(EVENT_PHASE_PUNCH, 15500);
                events.ScheduleEvent(EVENT_SUMMON_COLLAPSING_STAR, 18000);
                events.ScheduleEvent(EVENT_BIG_BANG, 90000);
                events.ScheduleEvent(EVENT_ASCEND_TO_THE_HEAVENS, 360000);
                events.ScheduleEvent(EVENT_COSMIC_SMASH, 25000);
                events.ScheduleEvent(EVENT_LIVING_CONSTELLATION , 50000);
                events.ScheduleEvent(EVENT_CHECK_EQUIP, 15000);
            }
            
            void JustSummoned(Creature* summon) override
            {
                summons.Summon(summon);
                switch (summon->GetEntry())
                {
                    case NPC_AZEROTH:
                        DoCastAOE(SPELL_REORIGINATION, true);
                        break;
                    case NPC_ALGALON_VOID_ZONE_VISUAL_STALKER:
                        summon->CastSpell(summon, SPELL_VOID_ZONE_VISUAL, TRIGGERED_FULL_MASK);
                        break;
                    case NPC_ALGALON_STALKER_ASTEROID_TARGET_01:
                        summon->CastSpell(summon, SPELL_COSMIC_SMASH_VISUAL_STATE, TRIGGERED_FULL_MASK);
                        break;
                    case NPC_ALGALON_STALKER_ASTEROID_TARGET_02:
                        summon->m_Events.AddEvent(new CosmicSmashDamageEvent(summon), summon->m_Events.CalculateTime(3250));
                        break;
                }
            }

            void EnterEvadeMode() override
            {
                instance->SetBossState(BOSS_ALGALON, FAIL);
                BossAI::EnterEvadeMode();
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                me->SetSheath(SHEATH_STATE_UNARMED);
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType) override
            {
                if (!_phaseTwo && me->HealthBelowPct(20))//Phase two
                {
                    _phaseTwo = true;
                    Talk(SAY_ALGALON_PHASE_TWO);
                    phase = PHASE_TWO;
                    summons.DespawnAll();
                    events.CancelEvent(EVENT_LIVING_CONSTELLATION);
                    events.CancelEvent(EVENT_SUMMON_COLLAPSING_STAR);
                    for (uint32 i = 0; i < 4; i++)
                        me->SummonCreature(32953, blackholepos[i], TEMPSUMMON_CORPSE_DESPAWN);
                }
            }
               
            void JustDied(Unit * killer) override
            {
                _JustDied();
                if (instance)
                    instance->DoUpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET, SPELL_KILL_CREDIT, 0, 0, me); //Observed

                me->setFaction(35);
                me->SetPhaseMask(65535, true);
                me->SummonGameObject(RAID_MODE(194821, 194822), 1632.53f, -295.983f, 417.323f, 1.56774f, 0, 0, 0.706026f, 0.708186f, 604800);
            }
            
            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return; 
                
                if (RemoveFactionAura)
                {
                    if (RemoveFactionAura <= diff)
                    {
                        instance->DoRemoveAurasDueToSpellOnPlayers(55773);
                        instance->DoRemoveAurasDueToSpellOnPlayers(55774);
                        RemoveFactionAura = 2000;
                    }
                    else RemoveFactionAura -= diff;
                }
                
                events.Update(diff);

                if (CheckHomeDistToEvade(diff, 50.0f))
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (phase == PHASE_ONE || phase == PHASE_TWO)
                {
                    while (uint32 eventId = events.ExecuteEvent())
                    {
                        switch (eventId)
                        {
                            case EVENT_QUANTUM_STRIKE:
                                DoCastVictim(SPELL_QUANTUM_STRIKE);
                                events.ScheduleEvent(EVENT_QUANTUM_STRIKE, urand(3000, 5000));
                                break;
                            case EVENT_PHASE_PUNCH:
                                DoCastVictim(SPELL_PHASE_PUNCH);
                                events.ScheduleEvent(EVENT_PHASE_PUNCH, 15500);
                                break;
                            case EVENT_SUMMON_COLLAPSING_STAR:
                                Talk(SAY_ALGALON_COLLAPSING_STAR);
                                Talk(EMOTE_ALGALON_COLLAPSING_STAR);
                                for (uint32 i = 0; i < COLLAPSING_STAR_COUNT; ++i)
                                    me->SummonCreature(NPC_COLLAPSING_STAR, CollapsingStarPos[i], TEMPSUMMON_MANUAL_DESPAWN);
                                events.ScheduleEvent(EVENT_SUMMON_COLLAPSING_STAR, 60000);
                                break;
                            case EVENT_LIVING_CONSTELLATION :
                                for (uint32 n = 0; n < 4; n++)
                                    me->SummonCreature(RAID_MODE(33052, 33116), ConstellationPos[rand()%6], TEMPSUMMON_CORPSE_DESPAWN);
                                events.ScheduleEvent(EVENT_LIVING_CONSTELLATION , 50000);
                                break;
                            case EVENT_BIG_BANG:
                            {
                                Talk(SAY_ALGALON_BIG_BANG);
                                Talk(EMOTE_ALGALON_BIG_BANG);
                                DoCastAOE(SPELL_BIG_BANG);
                                events.DelayEvents(9000);
                                events.ScheduleEvent(EVENT_BIG_BANG, 90500);
                                break;
                            }
                            case EVENT_COSMIC_SMASH:
                                Talk(EMOTE_ALGALON_COSMIC_SMASH);
                                DoCastAOE(SPELL_COSMIC_SMASH);
                                events.ScheduleEvent(EVENT_COSMIC_SMASH, 25500);
                                break;
                            case EVENT_CHECK_EQUIP:
                                instance->SetData(DATA_ALGALON, 0);
                                events.ScheduleEvent(EVENT_CHECK_EQUIP, 30000);
                                break;
                        }
                    }
                }
                DoMeleeAttackIfReady();
            }

        private:
            bool _firstPull;
            bool _fedOnTears;
            bool _phaseTwo;
            bool _fightWon;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetUlduarAI<boss_algalon_the_observerAI>(creature);
        }
};

class npc_living_constellation : public CreatureScript
{
    public:
        npc_living_constellation() : CreatureScript("npc_living_constellation") { }

        struct npc_living_constellationAI : public CreatureAI
        {
            npc_living_constellationAI(Creature* creature) : CreatureAI(creature)
            { 
                me->SetDisableGravity(false);
                me->SetCanFly(true);
                me->SetReactState(REACT_AGGRESSIVE);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->SetSpeed(MOVE_RUN, 0.5, true);
            }

            uint32 ArcaneBarrageTimer;
            uint32 CheckHeightTimer;
            float x,y,z,floor;

            void Reset() override
            {
                DoZoneInCombat();
                floor = 418.15f;
                ArcaneBarrageTimer = 3000;
                CheckHeightTimer = 1500;
            }
            
            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    me->DespawnOrUnsummon();

                if (ArcaneBarrageTimer <= diff)
                {
                    if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                        DoCast(target, SPELL_ARCANE_BARRAGE);
                    ArcaneBarrageTimer = 5000;
                }
                ArcaneBarrageTimer -= diff;

                if (CheckHeightTimer <= diff)
                {
                    CheckHeight();
                    CheckHeightTimer = 1500;
                }
                CheckHeightTimer -= diff;
            }

            void CheckHeight()
            {
                me->GetPosition(x,y,z);

                if (z <= floor)
                    me->GetMotionMaster()->MoveJump(me->GetPositionX(), me->GetPositionY(), 420.15f, 10, 10);

            }

        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetUlduarAI<npc_living_constellationAI>(creature);
        }
};

class npc_collapsing_star : public CreatureScript
{
    public:
        npc_collapsing_star() : CreatureScript("npc_collapsing_star") { }

        struct npc_collapsing_starAI : public PassiveAI
        {
            npc_collapsing_starAI(Creature* creature) : PassiveAI(creature)
            {
                _dying = false;
                me->SetCanFly(true);
                me->SetDisableGravity(true);
                me->SetReactState(REACT_PASSIVE);
                me->GetMotionMaster()->MoveRandom(10.0f);
                me->CastSpell(me, SPELL_COLLAPSE, TRIGGERED_FULL_MASK);
            }
            
            void DamageTaken(Unit * attacker, uint32 &damage, DamageEffectType dmgType) override
            {
                if (damage >= me->GetHealth() && !_dying)
                {
                    me->GetMotionMaster()->Initialize();
                    _dying = true;
                    me->RemoveAllAuras();
                    me->SetFullHealth();
                    damage = 0;
                    DoCast(me, SPELL_BLACK_HOLE_SPAWN_VISUAL, true);
                    DoCastAOE(SPELL_BLACK_HOLE_EXPLOSION);
                    Position pos;
                    me->GetPosition(&pos);
                    if (Unit * Algalon = me->ToTempSummon()->GetSummoner())
                        Algalon->SummonCreature(32953, pos, TEMPSUMMON_CORPSE_DESPAWN);
                    me->DespawnOrUnsummon(1000);
                }
            }

           bool _dying;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetUlduarAI<npc_collapsing_starAI>(creature);
        }
};

class npc_black_hole : public CreatureScript
{
    public:
        npc_black_hole() : CreatureScript("npc_black_hole") { }

        struct npc_black_holeAI : public CreatureAI
        {
            npc_black_holeAI(Creature* creature) : CreatureAI(creature)
            {
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

                if (Creature * Algalon = me->ToTempSummon()->GetSummoner()->ToCreature())
                    if (boss_algalon_the_observer::boss_algalon_the_observerAI * AlgalonAI = CAST_AI(boss_algalon_the_observer::boss_algalon_the_observerAI, Algalon->AI()))
                        if (!AlgalonAI->GetPhase())
                        {
                            SummonMatter = 0;
                            DoCast(me, 64469);
                            DoCast(me, SPELL_BLACK_HOLE_TRIGGER); 
                            DoCast(me, SPELL_CONSTELLATION_PHASE_TRIGGER);
                        }
                        else
                            SummonMatter = 3000;
                
            }

            uint32 SummonMatter;
            bool despawn;

            void Reset() override
            {
                despawn = false;
            }

            void UpdateAI(uint32 diff) override
            {
                if (SummonMatter)
                {
                    if (SummonMatter <= diff)
                    {
                        Position pos;
                        me->GetPosition(&pos);
                        if (Unit * Algalon = me->ToTempSummon()->GetSummoner()) 
                        {
                            if (Creature * matter = Algalon->SummonCreature(33089, pos, TEMPSUMMON_CORPSE_DESPAWN))
                            {
                                matter->AI()->DoZoneInCombat();
                                SummonMatter = 30000;
                            }
                        }
                    }
                    else SummonMatter -= diff;
                }
            }

            void SpellHitTarget(Unit * target, SpellInfo const* spell) override
            {
                if (spell->Id == 65509 && target->GetTypeId() == TYPEID_UNIT)
                {
                    if ((target->GetEntry() == 33052 || target->GetEntry() == 33116) && !despawn)
                    {
                        despawn = true;
                        me->Kill(target, true);
                        me->DespawnOrUnsummon(1000);
                        if (Creature * Algalon = me->ToTempSummon()->GetSummoner()->ToCreature())
                            Algalon->AI()->DoAction(ACTION_BLACK_HOLE_VAL);
                        
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetUlduarAI<npc_black_holeAI>(creature);
        }
};

class npc_dark_matter : public CreatureScript
{
    public:
        npc_dark_matter() : CreatureScript("npc_dark_matter") { }

        struct npc_dark_matterAI : public CreatureAI
        {
            npc_dark_matterAI(Creature* creature) : CreatureAI(creature)
            {
                me->GetMotionMaster()->MoveRandom(5.0f);
            }

            void Reset() override {}
 
            void UpdateAI(uint32 diff) override
            {
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetUlduarAI<npc_dark_matterAI>(creature);
        }
};

class npc_brann_bronzebeard_algalon : public CreatureScript //Algalon add in world, event not need
{
    public:
        npc_brann_bronzebeard_algalon() : CreatureScript("npc_brann_bronzebeard_algalon") { }

        struct npc_brann_bronzebeard_algalonAI : public CreatureAI
        {
            npc_brann_bronzebeard_algalonAI(Creature* creature) : CreatureAI(creature)
            {
            }

            void DoAction(int32 const action) override
            {
                switch (action)
                {
                    case ACTION_START_INTRO:
                        _currentPoint = 0;
                        _events.Reset();
                        me->SetWalk(false);
                        _events.ScheduleEvent(EVENT_BRANN_MOVE_INTRO, 1);
                        break;
                    case ACTION_FINISH_INTRO:
                        Talk(SAY_BRANN_ALGALON_INTRO_2);
                        _events.ScheduleEvent(EVENT_BRANN_MOVE_INTRO, 1);
                        break;
                    case ACTION_OUTRO:
                        me->GetMotionMaster()->MovePoint(POINT_BRANN_OUTRO, BrannOutroPos[1]);
                        _events.ScheduleEvent(EVENT_BRANN_OUTRO_1, 89500);
                        _events.ScheduleEvent(EVENT_BRANN_OUTRO_2, 116500);
                        break;
                }
            }

            void MovementInform(uint32 movementType, uint32 pointId) override
            {
                if (movementType != POINT_MOTION_TYPE)
                    return;

                uint32 delay = 1;
                _currentPoint = pointId + 1;
                switch (pointId)
                {
                    case 2:
                        delay = 8000;
                        me->SetWalk(true);
                        break;
                    case 5:
                        me->SetWalk(false);
                        Talk(SAY_BRANN_ALGALON_INTRO_1);
                        _events.ScheduleEvent(EVENT_SUMMON_ALGALON, 7500);
                        return;
                    case 9:
                        me->DespawnOrUnsummon(1);
                        return;
                    case POINT_BRANN_OUTRO:
                    case POINT_BRANN_OUTRO_END:
                        return;
                }

                _events.ScheduleEvent(EVENT_BRANN_MOVE_INTRO, delay);
            }

            void UpdateAI(uint32 diff) override
            {
                UpdateVictim();

                if (_events.Empty())
                    return;

                _events.Update(diff);

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_BRANN_MOVE_INTRO:
                            if (_currentPoint < MAX_BRANN_WAYPOINTS_INTRO)
                                me->GetMotionMaster()->MovePoint(_currentPoint, BrannIntroWaypoint[_currentPoint]);
                            break;
                        case EVENT_SUMMON_ALGALON:
                            if (Creature* algalon = me->GetMap()->SummonCreature(NPC_ALGALON, AlgalonSummonPos))
                                algalon->AI()->DoAction(ACTION_START_INTRO);
                            break;
                        case EVENT_BRANN_OUTRO_1:
                            Talk(SAY_BRANN_ALGALON_OUTRO);
                            break;
                        case EVENT_BRANN_OUTRO_2:
                            me->GetMotionMaster()->MovePoint(POINT_BRANN_OUTRO_END, BrannOutroPos[2]);
                            break;
                    }
                }
            }

        private:
            EventMap _events;
            uint32 _currentPoint;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetUlduarAI<npc_brann_bronzebeard_algalonAI>(creature);
        }
};

class go_celestial_planetarium_access : public GameObjectScript
{
    public:
        go_celestial_planetarium_access() : GameObjectScript("go_celestial_planetarium_access") {}

        struct go_celestial_planetarium_accessAI : public GameObjectAI
        {
            go_celestial_planetarium_accessAI(GameObject* go) : GameObjectAI(go)
            {
            }

            bool GossipHello(Player* player) override
            {
                // Start Algalon event
                go->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_IN_USE);
                /* _events.ScheduleEvent(EVENT_DESPAWN_CONSOLE, 5000);
                if (Creature* brann = go->SummonCreature(NPC_BRANN_BRONZBEARD_ALG, BrannIntroSpawnPos))
                    brann->AI()->DoAction(ACTION_START_INTRO); */

                if (InstanceScript* instance = go->GetInstanceScript())
                {
                    instance->SetData(DATA_ALGALON_SUMMON_STATE, 1);
                    if (GameObject* sigil = ObjectAccessor::GetGameObject(*go, instance->GetGuidData(GO_DOODAD_UL_SIGILDOOR_01)))
                        sigil->SetGoState(GO_STATE_ACTIVE);

                    if (GameObject* sigil = ObjectAccessor::GetGameObject(*go, instance->GetGuidData(GO_DOODAD_UL_SIGILDOOR_02)))
                        sigil->SetGoState(GO_STATE_ACTIVE);
                }

                return false;
            }

            void UpdateAI(uint32 diff) override
            {
                if (_events.Empty())
                    return;

                _events.Update(diff);

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_DESPAWN_CONSOLE:
                            go->Delete();
                            break;
                    }
                }
            }

            EventMap _events;
        };

        GameObjectAI* GetAI(GameObject* go) const override
        {
            return GetUlduarAI<go_celestial_planetarium_accessAI>(go);
        }
};

class spell_algalon_phase_punch : public SpellScriptLoader
{
    public:
        spell_algalon_phase_punch() : SpellScriptLoader("spell_algalon_phase_punch") { }

        class spell_algalon_phase_punch_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_algalon_phase_punch_AuraScript);

            void HandlePeriodic(AuraEffect const* /*aurEff*/)
            {
                PreventDefaultAction();
                if (GetStackAmount() != 1)
                    GetTarget()->RemoveAurasDueToSpell(PhasePunchAlphaId[GetStackAmount() - 2]);
                GetTarget()->CastSpell(GetTarget(), PhasePunchAlphaId[GetStackAmount() - 1], TRIGGERED_FULL_MASK);
                if (GetStackAmount() == 5)
                    Remove(AURA_REMOVE_BY_DEFAULT);
            }

            void OnRemove(AuraEffect const*, AuraEffectHandleModes)
            {
                if (GetStackAmount() != 5)
                    GetTarget()->RemoveAurasDueToSpell(PhasePunchAlphaId[GetStackAmount() - 1]);
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_algalon_phase_punch_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
                OnEffectRemove += AuraEffectRemoveFn(spell_algalon_phase_punch_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_algalon_phase_punch_AuraScript();
        }
};

class NotVictimFilter
{
    public:
        NotVictimFilter(Unit* caster) : _victim(caster->getVictim())
        {
        }

        bool operator()(WorldObject* target)
        {
            return target != _victim;
        }

    private:
        Unit* _victim;
};

class spell_algalon_arcane_barrage : public SpellScriptLoader
{
    public:
        spell_algalon_arcane_barrage() : SpellScriptLoader("spell_algalon_arcane_barrage") { }

        class spell_algalon_arcane_barrage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_algalon_arcane_barrage_SpellScript);

            void SelectTarget(std::list<WorldObject*>& targets)
            {
                targets.remove_if(NotVictimFilter(GetCaster()));
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_algalon_arcane_barrage_SpellScript::SelectTarget, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_algalon_arcane_barrage_SpellScript();
        }
};

class ActiveConstellationFilter
{
    public:
        bool operator()(WorldObject* target) const
        {
            if(Unit* unit = target->ToUnit())
                return unit->GetAI()->GetData(0);
            return false;
        }
};

class spell_algalon_trigger_3_adds : public SpellScriptLoader
{
    public:
        spell_algalon_trigger_3_adds() : SpellScriptLoader("spell_algalon_trigger_3_adds") { }

        class spell_algalon_trigger_3_adds_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_algalon_trigger_3_adds_SpellScript);

            void SelectTarget(std::list<WorldObject*>& targets)
            {
                targets.remove_if(ActiveConstellationFilter());
            }

            void HandleDummy(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                Creature* target = GetHitCreature();
                if (!target)
                    return;

                target->AI()->DoAction(ACTION_ACTIVATE_STAR);
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_algalon_trigger_3_adds_SpellScript::SelectTarget, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_algalon_trigger_3_adds_SpellScript();
        }
};

class spell_algalon_collapse : public SpellScriptLoader
{
    public:
        spell_algalon_collapse() : SpellScriptLoader("spell_algalon_collapse") { }

        class spell_algalon_collapse_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_algalon_collapse_AuraScript);

            void HandlePeriodic(AuraEffect const* /*aurEff*/)
            {
                PreventDefaultAction();
                GetTarget()->DealDamage(GetTarget(), GetTarget()->CountPctFromMaxHealth(1), NULL, NODAMAGE);
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_algalon_collapse_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_algalon_collapse_AuraScript();
        }
};

class spell_algalon_remove_phase : public SpellScriptLoader
{
    public:
        spell_algalon_remove_phase() : SpellScriptLoader("spell_algalon_remove_phase") { }

        class spell_algalon_remove_phase_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_algalon_remove_phase_AuraScript);

            void HandlePeriodic(AuraEffect const* /*aurEff*/)
            {
                PreventDefaultAction();
                GetTarget()->RemoveAurasByType(SPELL_AURA_PHASE);
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_algalon_remove_phase_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_algalon_remove_phase_AuraScript();
        }
};

class spell_algalon_cosmic_smash : public SpellScriptLoader
{
    public:
        spell_algalon_cosmic_smash() : SpellScriptLoader("spell_algalon_cosmic_smash") { }

        class spell_algalon_cosmic_smash_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_algalon_cosmic_smash_SpellScript);

            void ModDestHeight(SpellEffIndex /*effIndex*/)
            {
                Position offset = {0.0f, 0.0f, 65.0f, 0.0f};
                const_cast<WorldLocation*>(GetExplTargetDest())->RelocateOffset(offset);
                GetHitDest()->RelocateOffset(offset);
            }

            void Register() override
            {
                OnEffectLaunch += SpellEffectFn(spell_algalon_cosmic_smash_SpellScript::ModDestHeight, EFFECT_0, SPELL_EFFECT_SUMMON);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_algalon_cosmic_smash_SpellScript();
        }
};

class spell_algalon_cosmic_smash_damage : public SpellScriptLoader
{
    public:
        spell_algalon_cosmic_smash_damage() : SpellScriptLoader("spell_algalon_cosmic_smash_damage") { }

        class spell_algalon_cosmic_smash_damage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_algalon_cosmic_smash_damage_SpellScript);

            void RecalculateDamage()
            {
                if (!GetExplTargetDest() || !GetHitUnit())
                    return;

                float distance = GetHitUnit()->GetDistance2d(GetExplTargetDest()->GetPositionX(), GetExplTargetDest()->GetPositionY());
                if (distance > 6.0f)
                    SetHitDamage(int32(float(GetHitDamage()) / distance) * 2);
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_algalon_cosmic_smash_damage_SpellScript::RecalculateDamage);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_algalon_cosmic_smash_damage_SpellScript();
        }
};

class achievement_he_feeds_on_your_tears : public AchievementCriteriaScript
{
    public:
        achievement_he_feeds_on_your_tears() : AchievementCriteriaScript("achievement_he_feeds_on_your_tears") { }

        bool OnCheck(Player * player, Unit* target) override
        {
            if (!target)
                return false;

            if (Creature * Al = target->ToCreature())
                if (boss_algalon_the_observer::boss_algalon_the_observerAI * AlAI = CAST_AI(boss_algalon_the_observer::boss_algalon_the_observerAI, Al->AI()))
                    if (AlAI->GetfedOnTears())
                        return true;

            return false;
            
        }
};

void AddSC_boss_algalon_the_observer()
{
    new boss_algalon_the_observer();
    new npc_living_constellation();
    new npc_collapsing_star();
    new npc_black_hole();
    new npc_dark_matter();
    //new npc_brann_bronzebeard_algalon();
    new go_celestial_planetarium_access();
    new spell_algalon_phase_punch();
    new spell_algalon_arcane_barrage();
    new spell_algalon_trigger_3_adds();
    new spell_algalon_collapse();
    new spell_algalon_remove_phase();
    new spell_algalon_cosmic_smash();
    new spell_algalon_cosmic_smash_damage();
    //new achievement_he_feeds_on_your_tears();
}
