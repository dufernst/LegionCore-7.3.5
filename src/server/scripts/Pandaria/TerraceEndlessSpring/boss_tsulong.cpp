/*
 * Copyright (C) 2012-2013 JadeCore <http://www.pandashan.com/>
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

#include "GameObjectAI.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "terrace_of_endless_spring.h"
#include "GridNotifiers.h"
#include "ObjectVisitors.hpp"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"

enum eTsulongEvents
{
    EVENT_NONE,
    EVENT_FLY,
    EVENT_WAYPOINT_FIRST,
    EVENT_WAYPOINT_SECOND,
    EVENT_SWITCH_TO_NIGHT_PHASE,

    // night
    EVENT_SPAWN_SUNBEAM = 993,
    EVENT_SHADOW_BREATH,
    EVENT_NIGHTMARE,

    // day
    EVENT_SUN_BREATH,
    EVENT_SUMMON_UNSTABLE_SHA,
    EVENT_EMBODIED_TERROR,
    EVENT_WIN_DAY,
    EVENT_BERSERK,
};

enum eTsulongSpells
{
    SPELL_DREAD_SHADOWS = 122767,
    SPELL_DREAD_SHADOWS_DEBUFF = 122768,
    SPELL_SUNBEAM_DUMMY = 122782,
    SPELL_SUNBEAM_PROTECTION = 122789,
    SPELL_NIGHT_PHASE_EFFECT = 122841,
    SPELL_SHADOW_BREATH = 122752,
    SPELL_NIGHTMARE_EXPLOSION = 122777,
    SPELL_NIGHTMARE = 122770,
    SPELL_SHA_ACTIVE = 122438,
    // day
    SPELL_SUN_BREATH = 123105,//122855,
    SPELL_BATHED_IN_LIGHT = 122858,
    SPELL_SUMMON_UNSTABLE_SHA = 122953,
    SPELL_UNSTABLE_BOLT = 122881,
    SPELL_SUMMON_EMBODIED_TERROR = 122995,
    SPELL_TERRORIZE = 123011,
    SPELL_TERROIZE_TOULONG_DMG = 123012,
    SPELL_TERRORIZE_PLAYERS_DMG = 123018,
    SPELL_FRIGHTEN = 123036,
    SPELL_GOLDEN_ACTIVE_1 = 124176,
    SPELL_GOLDEN_ACTIVE_2 = 122453,
    SPELL_TSULONG_LOOT = 132183,
};
enum eTsulongTimers
{
    TIMER_FIRST_WAYPOINT = 5000, // 5 secs for test, live : 120 000
};
enum eTsulongPhase
{
    PHASE_NONE,
    PHASE_FLY,
    PHASE_DAY,
    PHASE_NIGHT
};
enum eTsulongTalk
{
    TSULONG_AGGRO = 1,
    TSULONG_DEATH = 2,
    TSULONG_DEATH_IN_DAY = 8,
    VO_TES_SERPENT_EVENT_DAYTONIGHT = 3,
    VO_TES_SERPENT_EVENT_NIGHTTODAY = 4,
    VO_TES_SERPENT_SLAY_DARK = 6,
    VO_TES_SERPENT_SLAY_LIGHT = 5,
    VO_TES_SERPENT_SPELL_NIGGHTMARE = 7,
};
enum eTsulongWaypoints
{
    WAYPOINT_FIRST = 10001,
    WAYPOINT_SECOND = 10002,
    WAYPOINT_DAY = 10003,
};

enum eTsulongDisplay
{
    DISPLAY_TSULON_NIGHT = 42532,
    DISPLAY_TSULON_DAY = 42533
};

enum eTsulongActions
{
    ACTION_SPAWN_SUNBEAM = 30,
    PHASE_DAY_ACTIVATION = 2,
    PHASE_NIGHT_ACTIVATION = 3,
    ACTION_SPAWN_EMBODIED = 4,
    ACTION_WIN_DAY_MODE = 5,
    ACTION_WIN_NIGHT_MODE = 6,
    ACTION_RESPAWN = 7,
    ACTION_WIN_DAY = 8,
    ACTION_WIN_NIGHT = 9,
};

enum eTsulongCreatures
{
    SUNBEAM_DUMMY_ENTRY = 62849,
    TSULONG_BOSS_ENTRY = 62442,
    NPC_FRIGHT_SPAWN = 62977,
    NPC_EMBODIED_TERROR = 62969,
    NPC_UNSTABLE_SHA = 62919,
    NPC_TSULONG_BOSS = 62442,
    NPC_BREATH_TRIGGER = 6534531,
};
enum eTsulongObjects
{
    CHEST_TSULONG_GOLD_NORMAL_10_man = 215356,
    CHEST_TSULONG_GOLD_HC_10_man = 215357,
    CHEST_TSULONG_GOLD_NORMAL_25_man = 212922,
    CHEST_TSULONG_GOLD_HC_25_man = 215355,
};


class TsulongDespawner : public BasicEvent
{
    public:
        explicit TsulongDespawner(Creature* creature) : _creature(creature)
        {
        }

        bool Execute(uint64 /*currTime*/, uint32 /*diff*/)
        {
            Trinity::CreatureWorker<TsulongDespawner> worker(_creature, *this);
            Trinity::VisitNearbyGridObject(_creature, 333.0f, worker);
            return true;
        }

        void operator()(Creature* creature) const
        {
            switch (creature->GetEntry())
            {
                case NPC_TSULONG_BOSS:
                    creature->RemoveAllAuras();
                    creature->SetHealth(creature->GetMaxHealth());
                    creature->GetMotionMaster()->MoveTargetedHome();
                    creature->SetVisible(true);
                    break;
                default:
                    return;
            }
        }

    private:
        Creature* _creature;
};

class boss_tsulong : public CreatureScript
{
public:
    boss_tsulong() : CreatureScript("boss_tsulong") { }

    struct boss_tsulongAI : public BossAI
    {
        boss_tsulongAI(Creature* creature) : BossAI(creature, DATA_TSULONG)
        {
            pInstance = creature->GetInstanceScript();
            won = false;
            me->Respawn();
        }

        InstanceScript* pInstance;
        EventMap events;

        int32 nightphasetimer;
        int32 dayphasetimer;
        uint8 phase;
        bool firstSpecialEnabled;
        bool secondSpecialEnabled;
        bool inFly;

        bool won;

        int32 energybar;
        int32 dayhealth;
        int32 nighthealth;

        int32 healtoreduce;
        int32 currenthealth;

        void Reset()
        {
            _Reset();
            events.Reset();
            summons.DespawnAll();

            me->RemoveUnitMovementFlag(MOVEMENTFLAG_ROOT);
            me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            inFly = false;
            won = false;
            me->SetDisableGravity(true);
            me->SetCanFly(true);
            me->RemoveAurasDueToSpell(SPELL_DREAD_SHADOWS);
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_OBS_MOD_HEALTH, true);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_HEAL_PCT, true);
            if (pInstance)
            {
                if (pInstance->GetBossState(DATA_PROTECTORS) == DONE)
                {
                    phase = PHASE_NIGHT;
                    me->SetDisplayId(DISPLAY_TSULON_NIGHT);
                    me->setFaction(14);
                    me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, 0);
                    me->SetHomePosition(-1017.841f, -3049.621f, 12.823f, 4.72f);
                    me->GetMotionMaster()->MoveTargetedHome();
                }
                else
                {
                    me->SetDisplayId(DISPLAY_TSULON_DAY);
                    me->setFaction(35);
                    me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, 35);
                }
            }
        }
        void EventReset()
        {
            events.CancelEvent(EVENT_SPAWN_SUNBEAM);
            events.CancelEvent(EVENT_SHADOW_BREATH);
            events.CancelEvent(EVENT_NIGHTMARE);
            events.CancelEvent(EVENT_SUN_BREATH);
            events.CancelEvent(EVENT_SUMMON_UNSTABLE_SHA);
            events.CancelEvent(EVENT_EMBODIED_TERROR);
        }
        void JustReachedHome()
        {
            _JustReachedHome();

            if (pInstance)
                pInstance->SetBossState(DATA_TSULONG, FAIL);
        }
        void KilledUnit(Unit* who)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
            {
                switch (phase)
                {
                case PHASE_DAY:
                    Talk(VO_TES_SERPENT_SLAY_DARK);
                    break;
                case PHASE_NIGHT:
                    Talk(VO_TES_SERPENT_SLAY_LIGHT);
                    break;
                }
            }
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            currenthealth = me->GetHealth();
            dayhealth = me->GetMaxHealth() - me->GetHealth();
        }

        void EnterCombat(Unit* attacker)
        {
            if (pInstance)
            {
                pInstance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                DoZoneInCombat();
            }

            pInstance->SetBossState(DATA_TSULONG, IN_PROGRESS);

            me->SetPower(POWER_ENERGY, 0);
            me->SetInt32Value(UNIT_FIELD_POWER, 0);
            me->SetMaxPower(POWER_ENERGY, 100);
            me->SetInt32Value(UNIT_FIELD_MAX_POWER, 100);
            me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_REGENERATE_POWER);

            Talk(TSULONG_AGGRO);

            dayphasetimer = 1.2 * IN_MILLISECONDS;

            phase = PHASE_NIGHT;
            events.SetPhase(PHASE_NIGHT);
            events.ScheduleEvent(EVENT_SWITCH_TO_NIGHT_PHASE, 0, 0, PHASE_NIGHT);
            events.ScheduleEvent(EVENT_SPAWN_SUNBEAM, 2000, 0, PHASE_NIGHT);
            events.ScheduleEvent(EVENT_SHADOW_BREATH, 30 * IN_MILLISECONDS, 0, PHASE_NIGHT);
            events.ScheduleEvent(EVENT_NIGHTMARE, 15 * IN_MILLISECONDS, 0, PHASE_NIGHT);
            events.ScheduleEvent(EVENT_BERSERK, 480 * IN_MILLISECONDS);
        }
        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
        }
        void SummonedCreatureDespawn(Creature* summon)
        {
            summons.Despawn(summon);
        }
        void RegeneratePower(Powers power, float& value) override
        {
            if (power != POWER_ENERGY)
                return;

            if (phase == PHASE_DAY || phase == PHASE_NIGHT && me->isInCombat())
            {
                // Sha of Fear regenerates 6 energy every 2s (15 energy for 5s)
                if (phase == PHASE_NIGHT)
                    value = 2;
                else if (phase == PHASE_DAY)
                    value = 2;

                int32 val = me->GetPower(POWER_ENERGY);
                if (val + value > 100)
                    val = 100;
                else
                    val += value;

                me->SetInt32Value(UNIT_FIELD_POWER, val);
            }
        }
        void MoveInLineOfSight(Unit* who)
        {
            if (who && who->IsInWorld() && who->GetTypeId() != TYPEID_PLAYER)
            {
                switch (who->GetEntry())
                {
                    case 62919: // unstable sha ooze mechanique. [Heal, DMG]
                        if (me->IsWithinDistInMap(who, 2.0f, true))
                        {
                            if (phase == PHASE_DAY)
                            {
                                who->CastSpell(me, 123697, true);
                            }
                            else if (phase == PHASE_NIGHT)
                            {
                                who->CastSpell(me, 130078, true);
                            }
                            who->ToCreature()->DespawnOrUnsummon(1000);
                        }
                        break;
                }
            }
        }
        void JustDied(Unit* killer)
        {
            if (pInstance)
            {
                pInstance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                if (phase == PHASE_DAY) // PHASE DAY DEFEAT - WIPE
                {
                    summons.DespawnAll();
                    me->Respawn();
                    Reset();
                    EnterEvadeMode();
                    EventReset();

                    Talk(TSULONG_DEATH);
                    pInstance->SetBossState(DATA_TSULONG, FAIL);
                }
                else if (phase == PHASE_NIGHT) // PHASE NIGHT DEFEAT - win
                {
                    summons.DespawnAll();

                    DespawnCreaturesInArea(NPC_EMBODIED_TERROR, me);
                    DespawnCreaturesInArea(NPC_UNSTABLE_SHA, me);
                    DespawnCreaturesInArea(NPC_FRIGHT_SPAWN, me);

                    events.CancelEvent(EVENT_EMBODIED_TERROR);
                    events.CancelEvent(EVENT_SUMMON_UNSTABLE_SHA);
                    events.CancelEvent(EVENT_SUN_BREATH);

                    /*
                    NO LOOT CHEST IF DIED ON NIGHT PHASE
                    */

                    pInstance->SetBossState(DATA_TSULONG, DONE);
                    Talk(TSULONG_DEATH);

                    _JustDied();
                }
            }
        }
        void DoAction(const int32 action)
        {
            if (action == ACTION_START_TSULONG_WAYPOINT)
            {
                std::list<Creature*> fear;
                std::list<Creature*> terror;
                me->GetCreatureListWithEntryInGrid(fear, NPC_APPARITION_OF_FEAR, 150.0f);
                me->GetCreatureListWithEntryInGrid(fear, NPC_APPARITION_OF_TERROR, 150.0f);

                for (auto itr : terror)
                {
                    if (itr && itr->IsInWorld())
                    {
                        itr->DespawnOrUnsummon();
                    }
                }
                for (auto itr : fear)
                {
                    if (itr && itr->IsInWorld())
                    {
                        itr->DespawnOrUnsummon();
                    }
                }

                inFly = true;

                phase = PHASE_FLY;
                events.SetPhase(phase);
                events.ScheduleEvent(EVENT_FLY, 5000, 0, phase);
            }

            if (action == ACTION_SPAWN_SUNBEAM)
                events.ScheduleEvent(EVENT_SPAWN_SUNBEAM, 5000, 0, PHASE_NIGHT);

            if (action == ACTION_WIN_DAY)
                events.ScheduleEvent(EVENT_WIN_DAY, 500);

            if (action == PHASE_DAY_ACTIVATION)
            {
                phase = PHASE_DAY;
                Talk(VO_TES_SERPENT_EVENT_NIGHTTODAY);
                events.SetPhase(phase);

                events.ScheduleEvent(EVENT_EMBODIED_TERROR, 40 * IN_MILLISECONDS, 0, PHASE_DAY);
                events.ScheduleEvent(EVENT_SUN_BREATH, 30 * IN_MILLISECONDS, 0, PHASE_DAY);
                events.ScheduleEvent(EVENT_SUMMON_UNSTABLE_SHA, 15 * IN_MILLISECONDS, 0, PHASE_DAY);

                me->AttackStop();

                events.ScheduleEvent(WAYPOINT_DAY, 100, 0, PHASE_DAY);

                std::list<Creature*> sunbeamtodespawn;

                GetCreatureListWithEntryInGrid(sunbeamtodespawn, me, SUNBEAM_DUMMY_ENTRY, 300.0F);

                for (auto sunbeam : sunbeamtodespawn)
                {
                    sunbeam->DespawnOrUnsummon();
                }      
               
                me->CastSpell(me, SPELL_GOLDEN_ACTIVE_1);
                me->CastSpell(me, SPELL_GOLDEN_ACTIVE_2);
                me->RemoveAura(SPELL_GOLDEN_ACTIVE_1);
                me->RemoveAura(SPELL_GOLDEN_ACTIVE_2);
                me->SetDisplayId(DISPLAY_TSULON_DAY);
                me->setFaction(1665);
                //me->RemoveAura(SPELL_SHA_ACTIVE);
                me->SetHealth(dayhealth);
                me->AddUnitMovementFlag(MOVEMENTFLAG_ROOT);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);

                me->SetPower(POWER_ENERGY, 0);
                me->SetInt32Value(UNIT_FIELD_POWER, 0);
                me->SetMaxPower(POWER_ENERGY, 100);
                me->SetInt32Value(UNIT_FIELD_MAX_POWER, 100);
                me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_REGENERATE_POWER);
            }
            if (action == PHASE_NIGHT_ACTIVATION)
            {               
                phase = PHASE_NIGHT;
                Talk(VO_TES_SERPENT_EVENT_DAYTONIGHT);
                events.SetPhase(phase);

                me->GetMotionMaster()->Clear();
                me->Attack(me->getVictim(), true);
                
   
                me->SetDisplayId(DISPLAY_TSULON_NIGHT);

                me->setFaction(16);
                me->SetHealth(nighthealth);
                me->SetSpeed(MOVE_RUN, 1.12f, true);   
                me->SetDisplayId(DISPLAY_TSULON_NIGHT);
                me->SetReactState(REACT_AGGRESSIVE);
                events.ScheduleEvent(EVENT_SPAWN_SUNBEAM, 2000, 0, PHASE_NIGHT);
                events.ScheduleEvent(EVENT_SHADOW_BREATH, 30 * IN_MILLISECONDS, 0, PHASE_NIGHT);
                events.ScheduleEvent(EVENT_NIGHTMARE, 15 * IN_MILLISECONDS, 0, PHASE_NIGHT);

                me->RemoveUnitMovementFlag(MOVEMENTFLAG_ROOT);
                me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);

                me->SetPower(POWER_ENERGY, 0);
                me->SetInt32Value(UNIT_FIELD_POWER, 0);
                me->SetMaxPower(POWER_ENERGY, 100);
                me->SetInt32Value(UNIT_FIELD_MAX_POWER, 100);
                me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_REGENERATE_POWER);

            }
            if (action == SPELL_SUMMON_EMBODIED_TERROR)
            {
                events.ScheduleEvent(SPELL_SUMMON_EMBODIED_TERROR, 30 * IN_MILLISECONDS, 0, PHASE_DAY);
            }
            if (action == ACTION_RESPAWN)
            {
                me->m_Events.AddEvent(new TsulongDespawner(me), me->m_Events.CalculateTime(5000));
            }
        }
        void MovementInform(uint32 type, uint32 id)
        {
            if (type != POINT_MOTION_TYPE)
                return;

            switch (id)
            {
                case WAYPOINT_FIRST:
                    events.ScheduleEvent(EVENT_WAYPOINT_FIRST, 0, 0, PHASE_FLY);
                    break;
                case WAYPOINT_SECOND:
                    events.ScheduleEvent(EVENT_WAYPOINT_SECOND, 0, 0, PHASE_FLY);
                    break;
                case WAYPOINT_DAY:
                    me->SetSpeed(MOVE_RUN, 0.0f);
                    break;
                default:
                    break;
            }
        }
        void DespawnCreaturesInArea(uint32 entry, WorldObject* object)
        {
            std::list<Creature*> creatures;
            GetCreatureListWithEntryInGrid(creatures, object,entry, 300.0f);
            if (creatures.empty())
                return;

            for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
                (*iter)->DespawnOrUnsummon();
        }
        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);
            
            if (phase == PHASE_FLY)
            {
                switch (events.ExecuteEvent())
                {
             
                case EVENT_FLY:
                    me->setFaction(14);
                    me->SetReactState(REACT_PASSIVE);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                    me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, 0);
                    me->SetDisplayId(DISPLAY_TSULON_NIGHT);
                    me->GetMotionMaster()->MovePoint(WAYPOINT_FIRST, -1018.10f, -2947.431f, 50.12f);
                    inFly = true;
                    break;
                case EVENT_WAYPOINT_FIRST:
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MovePoint(WAYPOINT_SECOND, -1017.841f, -3049.621f, 12.823f);
                    break;
                case EVENT_WAYPOINT_SECOND:
                    me->SetHomePosition(-1017.841f, -3049.621f, 12.823f, 4.72f);
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                    inFly = false;
                    events.SetPhase(PHASE_NONE);
                    phase = PHASE_NONE;
                    break;
                default:
                    break;
                }
            }
            if (UpdateVictim())
            {
                if (phase == PHASE_NIGHT)
                {
                    if (me->GetUInt32Value(UNIT_FIELD_POWER) == 100)
                    {
                        me->SetInt32Value(UNIT_FIELD_POWER, 0);

                        switch (phase)
                        {
                        case PHASE_NIGHT:
                            me->GetAI()->DoAction(PHASE_DAY_ACTIVATION);
                            break;
                        }
                    }

                    switch (events.ExecuteEvent())
                    {
                        case EVENT_BERSERK:
                            me->CastSpell(me, 26662);
                            break;
                        case EVENT_SWITCH_TO_NIGHT_PHASE:
                        {
                            me->SetDisplayId(DISPLAY_TSULON_NIGHT);
                            me->setFaction(14);
                            me->CastSpell(me, SPELL_DREAD_SHADOWS, true);
                            break;
                        }
                        case EVENT_SPAWN_SUNBEAM:
                        {
                            Position pos;
                            me->MonsterTextEmote("Tsulong casts |cffff0000[Sunbeam]|cfffaeb00!", me->GetGUID(), true);
                            me->GetRandomNearPosition(pos, 10.0f);
                            me->SummonCreature(SUNBEAM_DUMMY_ENTRY, pos);
                            break;
                        }
                        case EVENT_SHADOW_BREATH:
                            me->CastSpell(me->getVictim(), SPELL_SHADOW_BREATH);
                            events.ScheduleEvent(EVENT_SHADOW_BREATH, 28 * IN_MILLISECONDS, 0, PHASE_NIGHT);
                            break;
                        case EVENT_NIGHTMARE:
                        {
                            if (me->GetMap()->Is25ManRaid())
                            {
                                for (int i = 0; i <= 3; i++)
                                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0, true))
                                        me->CastSpell(target, SPELL_NIGHTMARE, true);
                            }
                            else
                            {
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0, true))
                                    me->CastSpell(target, SPELL_NIGHTMARE, true);
                            }
                            Talk(VO_TES_SERPENT_SPELL_NIGGHTMARE);
                            events.ScheduleEvent(EVENT_NIGHTMARE, 15 * IN_MILLISECONDS, 0, PHASE_NIGHT);
                            break;
                        }
                        default:
                            break;
                    }
                }
            }       
            if (phase == PHASE_DAY)
            {
                if (!won)
                {
                    nighthealth = me->GetMaxHealth() - me->GetHealth();

                    if (me->GetUInt32Value(UNIT_FIELD_POWER) == 100)
                    {
                        me->SetInt32Value(UNIT_FIELD_POWER, 0);

                        switch (phase)
                        {
                        case PHASE_DAY:
                            me->GetAI()->DoAction(PHASE_NIGHT_ACTIVATION);
                            break;
                        }
                    }
                }
                if (me->HealthAbovePct(98))
                {
                    me->GetAI()->DoAction(ACTION_WIN_DAY);
                }
                else
                {
                    Map::PlayerList const& players = me->GetMap()->GetPlayers();
                    if (!players.isEmpty())
                    {
                        for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                        {
                            if (Player* player = itr->getSource())
                            {
                                me->SetInCombatWith(player);
                                player->SetInCombatWith(me);
                                me->AddThreat(player, 5.0f);
                            }
                        }
                    }
                }

                switch (events.ExecuteEvent())
                {
                    case EVENT_WIN_DAY:
                    {
                        if (!pInstance)
                            return;

                        events.SetPhase(PHASE_NIGHT);
                        phase = PHASE_NIGHT;

                        won = true;
                        summons.DespawnAll();
                        Talk(TSULONG_DEATH_IN_DAY);
                        _JustDied();
                        me->AttackStop();           
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);       
                        pInstance->SetBossState(DATA_TSULONG, DONE);

                        DespawnCreaturesInArea(NPC_EMBODIED_TERROR, me);
                        DespawnCreaturesInArea(NPC_UNSTABLE_SHA, me);
                        DespawnCreaturesInArea(NPC_FRIGHT_SPAWN, me);

                        events.CancelEvent(EVENT_EMBODIED_TERROR);
                        events.CancelEvent(EVENT_SUMMON_UNSTABLE_SHA);
                        events.CancelEvent(EVENT_SUN_BREATH);

                        // HANDLE LOOT
                        if (me->GetMap()->IsHeroic())
                            me->SummonGameObject(CHEST_TSULONG_GOLD_HC_10_man, -1026.27f, -3033.25f, 12.689f, 1.612372f, 0, 0, 0, 0, 0);
                        else
                            me->SummonGameObject(CHEST_TSULONG_GOLD_NORMAL_10_man, -1026.27f, -3033.25f, 12.689f, 1.612372f, 0, 0, 0, 0, 0);
                        break;
                    }
                    case WAYPOINT_DAY:
                        me->GetMotionMaster()->MovePoint(WAYPOINT_DAY, -1009.97f, -3050.18f, 12.824f);
                        break;
                    case EVENT_BERSERK:
                        me->CastSpell(me, 26662);
                        break;
                    case EVENT_SUN_BREATH:
                    {
                        if (Creature* breath_trigger = me->FindNearestCreature(NPC_BREATH_TRIGGER, 200.0f, true))
                        {
                            me->SetFacingToObject(breath_trigger);
                            me->CastSpell(breath_trigger, SPELL_SUN_BREATH);
                        }

                        events.ScheduleEvent(EVENT_SUN_BREATH, 30 * IN_MILLISECONDS, 0, PHASE_DAY);

                        DoCastAOE(SPELL_BATHED_IN_LIGHT, true);

                        std::list<Player*> cone_D;
                        std::list<Creature*> cone_F;

                        me->GetCreatureListWithEntryInGrid(cone_F, 62969, 80.0f);

                        for (auto _s : cone_F)
                        {
                            if (me->isInFront(_s))
                            {
                                _s->DespawnOrUnsummon(2000);

                                // summon fright spawns
                                for (int i = 0; i < 5; i++)
                                {
                                    Position pos;
                                    _s->GetRandomNearPosition(pos, 4.0f);

                                    _s->SummonCreature(NPC_FRIGHT_SPAWN, pos);
                                }
                            }
                        }

                        Trinity::AnyPlayerInObjectRangeCheck u_check(me, 40.0f);
                        Trinity::PlayerListSearcher <Trinity::AnyPlayerInObjectRangeCheck> searcher(me, cone_D, u_check);
                        me->VisitNearbyObject(40.0f, searcher);

                        for (std::list<Player*>::const_iterator it = cone_D.begin(); it != cone_D.end(); ++it)
                        {
                            if (!(*it))
                                return;

                            if (me->isInFront((*it)))
                                me->AddAura(SPELL_BATHED_IN_LIGHT, (*it));
                        }


                        break;
                    }
                    case EVENT_SUMMON_UNSTABLE_SHA:
                    {
                        int x = 4;
                        int stacks = 0;
                        std::list<Creature*> unstable_sha_targets;
                        unstable_sha_targets.clear();

                        me->GetCreatureListWithEntryInGrid(unstable_sha_targets, 62962, 300.0f);

                        for (auto it : unstable_sha_targets)
                        {
                            me->CastSpell(it, SPELL_SUMMON_UNSTABLE_SHA, true);

                            stacks++;

                            if (stacks == x)
                                break;
                        }

                        events.ScheduleEvent(EVENT_SUMMON_UNSTABLE_SHA, 20 * IN_MILLISECONDS, 0, PHASE_DAY);
                        break;
                    }
                    case EVENT_EMBODIED_TERROR:
                    {
                        Position pos;
                        me->GetRandomNearPosition(pos, urand(15.0f, 25.0f));
                        me->CastSpell(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), SPELL_SUMMON_EMBODIED_TERROR, true);
                        events.ScheduleEvent(EVENT_EMBODIED_TERROR, 40 * IN_MILLISECONDS, 0, PHASE_DAY);
                        break;
                    }
                }

                // wiper 
                if (!me->isInCombat())
                    return;

                std::list<HostileReference*> const& threatList = me->getThreatManager().getThreatList();
                if (threatList.empty())
                {
                    EnterEvadeMode();
                    return;
                }

                // check evade every second tick
                _evadeCheck ^= true;
                if (!_evadeCheck)
                    return;

                // check if there is any player on threatlist, if not - evade
                for (std::list<HostileReference*>::const_iterator itr = threatList.begin(); itr != threatList.end(); ++itr)
                    if (Unit* target = (*itr)->getTarget())
                        if (target->GetTypeId() == TYPEID_PLAYER)
                            return; // found any player, return

                EnterEvadeMode();
                EventReset();
                summons.DespawnAll();
                me->SetVisible(false);
                me->GetAI()->DoAction(ACTION_RESPAWN);

            }

            DoMeleeAttackIfReady();
        }


    private:
        bool _evadeCheck;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_tsulongAI(creature);
    }
};
class npc_sunbeam : public CreatureScript
{
public:
    npc_sunbeam() : CreatureScript("npc_sunbeam") { }

    struct npc_sunbeamAI : public CreatureAI
    {
        InstanceScript* pInstance;

        int stacks = 0;

        npc_sunbeamAI(Creature* creature) : CreatureAI(creature)
        {
            pInstance = creature->GetInstanceScript();
            me->Respawn();
            creature->SetObjectScale(5.0f);
            creature->SetReactState(REACT_PASSIVE);
            creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
            creature->CastSpell(creature, SPELL_SUNBEAM_DUMMY, true);
        }
        void UpdateAI(uint32 const diff)
        {
            
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_sunbeamAI(creature);
    }
};
class npc_frigten_spawn : public CreatureScript
{
public:
    npc_frigten_spawn() : CreatureScript("npc_frigten_spawn") { }

    struct npc_frigten_spawnAI : public ScriptedAI
    {
        InstanceScript* pInstance;

        npc_frigten_spawnAI(Creature* creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
            me->Respawn();
        }
        uint32 frighten;

        void Reset()
        {
            frighten = 6000;
            me->setFaction(16);
            me->SetReactState(REACT_AGGRESSIVE);
            
            if (!Is25ManRaid())
            {
                me->SetMaxHealth(626064);
                me->SetHealth(626064);       
            }
            else
            {
                me->SetMaxHealth(626064);
                me->SetHealth(626064);
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (frighten <= diff)
            {
                if (Unit* random = SelectTarget(SELECT_TARGET_RANDOM, 0, 0, true))
                    me->CastSpell(random, SPELL_FRIGHTEN);

                frighten = 6000;
            }
            else
                frighten -= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_frigten_spawnAI(creature);
    }
};

class npc_unstable_sha : public CreatureScript
{
public:
    npc_unstable_sha() : CreatureScript("npc_unstable_sha") { }

    struct npc_npc_unstable_shaAI : public ScriptedAI
    {
        InstanceScript* pInstance;

        npc_npc_unstable_shaAI(Creature* creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
            creature->SetReactState(REACT_PASSIVE);
            me->Respawn();
        }
        uint32 unstablebolt;

        void Reset()
        {
            me->SetSpeed(MOVE_RUN, 0.5f, true);

            DoZoneInCombat();
            unstablebolt = 400;
            
            if (me->GetMap()->Is25ManRaid())
            {
                me->SetMaxHealth(1032900);
                me->SetHealth(1032900);
            }
            else
            {
                me->SetMaxHealth(929610);
                me->SetHealth(929610);
            }
        }
        void MoveTowardTsulong()
        {
            if (MotionMaster* motion = me->GetMotionMaster())
                if (Creature* tsulon = pInstance->instance->GetCreature(pInstance->GetGuidData(NPC_TSULONG_BOSS)))
                    if (tsulon && tsulon->IsInWorld())
                        motion->MovePoint(1, tsulon->GetPositionX(), tsulon->GetPositionY(), tsulon->GetPositionZ());
        }

        void UpdateAI(uint32 const diff)
        {
            if (unstablebolt <= diff)
            {
                if (Unit* random = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                    me->CastSpell(random, SPELL_UNSTABLE_BOLT);

                unstablebolt = 2000;
            }
            else
                unstablebolt -= diff;

            if (!me->isMoving())
                MoveTowardTsulong();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_npc_unstable_shaAI(creature);
    }
};

class npc_embodied_terror : public CreatureScript
{
public:
    npc_embodied_terror() : CreatureScript("npc_embodied_terror") { }

    struct npc_embodied_terrorAI : public ScriptedAI
    {
        InstanceScript* pInstance;

        npc_embodied_terrorAI(Creature* creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
            me->Respawn();
        }

        uint32 terrorize;

        void Reset()
        {
            terrorize = 6000;
            DoZoneInCombat();
            me->SetReactState(REACT_AGGRESSIVE);
            
            if (Is25ManRaid())
            {
                me->SetMaxHealth(11130029);
                me->SetHealth(11130029);
            }
            else
            {
                me->SetMaxHealth(3339009);
                me->SetHealth(3339009);
            }
        }
        void JustDied(Unit* killer)
        {
            // summon fright spawns
            for (int i = 0; i < 5; i++)
            {
                Position pos;
                me->GetRandomNearPosition(pos, 4.0f);

                me->SummonCreature(NPC_FRIGHT_SPAWN, pos);
            }
            if (Creature* tsulon = pInstance->instance->GetCreature(pInstance->GetGuidData(NPC_TSULONG_BOSS)))
            {
                tsulon->GetAI()->DoAction(ACTION_SPAWN_EMBODIED);
            }
        }
        void UpdateAI(uint32 const diff)
        {
            if (terrorize <= diff)
            {
                if (Unit* random = SelectTarget(SELECT_TARGET_RANDOM, 0, 20, true))
                    me->CastSpell(random, SPELL_TERRORIZE);

                if (pInstance)
                {
                    if (Creature* tsulon = pInstance->instance->GetCreature(pInstance->GetGuidData(NPC_TSULONG_BOSS)))
                    {
                        if (tsulon && tsulon->IsInWorld() && tsulon->getFaction() == 1665)
                        {
                            me->AddAura(SPELL_TERROIZE_TOULONG_DMG, tsulon);
                        }
                    }
                }

                terrorize = 15000;
            }
            else
                terrorize -= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_embodied_terrorAI(creature);
    }
};
// 125843, jam spell ?
class spell_dread_shadows_damage : public SpellScriptLoader
{
public:
    spell_dread_shadows_damage() : SpellScriptLoader("spell_dread_shadows_damage") { }

    class spell_dread_shadows_damage_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dread_shadows_damage_SpellScript);

        void RemoveInvalidTargets(std::list<WorldObject*>& targets)
        {
            targets.clear();
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dread_shadows_damage_SpellScript::RemoveInvalidTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dread_shadows_damage_SpellScript::RemoveInvalidTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_dread_shadows_damage_SpellScript();
    }
};

class DreadShadowsTargetCheck
{
public:
    bool operator()(WorldObject* object) const
    {
        // check Sunbeam protection
        if (object->ToUnit() && object->ToUnit()->HasAura(122789))
            return true;

        return false;
    }
};
// 122768
class spell_dread_shadows_malus : public SpellScriptLoader
{
public:
    spell_dread_shadows_malus() : SpellScriptLoader("spell_dread_shadows_malus") { }

    class spell_dread_shadows_malus_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dread_shadows_malus_SpellScript);

        void RemoveInvalidTargets(std::list<WorldObject*>& targets)
        {
            targets.remove(GetCaster());
            targets.remove_if(DreadShadowsTargetCheck());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dread_shadows_malus_SpellScript::RemoveInvalidTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dread_shadows_malus_SpellScript::RemoveInvalidTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_dread_shadows_malus_SpellScript();
    }
};

class OrientationCheck
{
public:
    explicit OrientationCheck(WorldObject* _caster) : caster(_caster) { }
    bool operator() (WorldObject* unit)
    {
        if (unit->isInFront(caster, 8.5f) && unit->GetTypeId() != TYPEID_PLAYER)
            return false;

        return true;
    }

private:
    WorldObject* caster;
};

// 122789
class spell_sunbeam : public SpellScriptLoader
{
public:
    spell_sunbeam() : SpellScriptLoader("spell_sunbeam") { }

    class spell_sunbeam_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_sunbeam_SpellScript);

        void CheckTargets(std::list<WorldObject*>& targets)
        {
            if (!GetCaster())
                return;

            targets.clear();
            Map::PlayerList const& players = GetCaster()->GetMap()->GetPlayers();
            for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
            {
                Player* plr = itr->getSource();
                if (!plr)
                    continue;

                float scale = GetCaster()->GetFloatValue(OBJECT_FIELD_SCALE);
                if (plr->GetExactDist2d(GetCaster()) <= scale)
                    targets.push_back(plr);
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sunbeam_SpellScript::CheckTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ALLY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_sunbeam_SpellScript();
    }


    class spell_sunbeam_aura_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_sunbeam_aura_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetCaster() && GetTarget())
            {
                if (GetTarget()->GetTypeId() == TYPEID_PLAYER)
                {
                    if (Pet* pet = GetTarget()->ToPlayer()->GetPet())
                        pet->AddAura(SPELL_SUNBEAM_PROTECTION, pet);

                    GetCaster()->CastSpell(GetCaster(), 52635);

                    if (GetCaster()->HasAura(52635))
                    {
                        if (auto ptr = GetCaster()->GetAura(52635))
                        {
                            if (ptr->GetStackAmount() > 5)
                            {
                                float scale = GetCaster()->GetFloatValue(OBJECT_FIELD_SCALE);
                                GetCaster()->SetObjectScale(scale - 0.8f);

                                if (Creature* tsulon = GetCaster()->FindNearestCreature(NPC_TSULONG_BOSS, 300.0f, true))
                                {
                                    tsulon->GetAI()->DoAction(ACTION_SPAWN_SUNBEAM);
                                    GetCaster()->ToCreature()->DespawnOrUnsummon();
                                }
                            }
                        }
                    }
                }
            }

            GetTarget()->RemoveAurasDueToSpell(SPELL_DREAD_SHADOWS_DEBUFF);
        }

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetTarget()->GetTypeId() == TYPEID_PLAYER)
            {
                if (Pet* pet = GetTarget()->ToPlayer()->GetPet())
                    pet->RemoveAurasDueToSpell(SPELL_SUNBEAM_PROTECTION);
            }
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_sunbeam_aura_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_sunbeam_aura_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_sunbeam_aura_AuraScript();
    }
};
class terroize_creature_restriction : public SpellScriptLoader
{
public:
    terroize_creature_restriction() : SpellScriptLoader("terroize_creature_restriction") { }

    class terroize_creature_restriction_SpellScript : public SpellScript
    {
        PrepareSpellScript(terroize_creature_restriction_SpellScript);

        SpellCastResult CheckTargetType()
        {
            if (GetExplTargetUnit() && GetExplTargetUnit()->GetTypeId() == TYPEID_PLAYER)
                return SPELL_CAST_OK;
            else
                return SPELL_FAILED_DONT_REPORT;
        }

        void Register()
        {
            OnCheckCast += SpellCheckCastFn(terroize_creature_restriction_SpellScript::CheckTargetType);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new terroize_creature_restriction_SpellScript();
    }
};

// terrorize player  - SPELL_TERRORIZE 
class spell_player_terrorize : public SpellScriptLoader
{
public:
    spell_player_terrorize() : SpellScriptLoader("spell_player_terrorize") { }

    class spell_player_terrorize_spellscript : public SpellScript
    {
        PrepareSpellScript(spell_player_terrorize_spellscript);

        void CorrectRange(std::list<WorldObject*>& targets)
        {
            Trinity::Containers::RandomResizeList(targets, GetCaster()->GetMap()->Is25ManRaid() ? 5 : 3);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_player_terrorize_spellscript::CorrectRange, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_player_terrorize_spellscript();
    }
};

void AddSC_boss_tsulong()
{
    // Boss
    new boss_tsulong();
    // Triggers
    new npc_sunbeam();
    // Spells
    new spell_dread_shadows_damage();
    new spell_dread_shadows_malus();
    new spell_sunbeam();
    new terroize_creature_restriction();
    new spell_player_terrorize();
    // Adds
    new npc_embodied_terror();
    new npc_unstable_sha();
    new npc_frigten_spawn();
}
