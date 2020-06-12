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

#include "heart_of_fear.h"
#include "MoveSplineInit.h"
#include "Vehicle.h"

enum eSpells
{
    SPELL_OVERWHELMING_ASSAULT  = 123474,
    SPELL_INTENSIFY             = 123470,
    SPELL_INTENSIFY_PHASE_TWO   = 132254,
    SPELL_INTENSIFY_TRIGGER_EF  = 123471,
    SPELL_TEMPEST_SLASH_SUM     = 122842,
    SPELL_TEMPEST_SLASH_DMG     = 122854,
    SPELL_WIND_STEP             = 123180,
    SPELL_UNSEEN_STRIKE_DMG     = 122994,
    SPELL_UNSEEN_STRIKE_TARGET  = 123017,
    //Phase 2
    SPELL_STORM_UNLEASHED_JUMP  = 123805,
    SPELL_STORM_UNLEASHED_VIS_1 = 123814, // 62543
    SPELL_STORM_UNLEASHED_VIS_2 = 124024, // 63567
    SPELL_STORM_UNLEASHED_DMG   = 124785,
    SPELL_STORM_UNLEASHED_AURA  = 123815,
    SPELL_RIDE_VEHICLE          = 124026,
    SPELL_RIDE_VEHICLE_2        = 123598,
    //Heroic
    SPELL_BLADE_TEMPEST_JUMP    = 125325,
    SPELL_BLADE_TEMPEST         = 125310,
    //Other
    //SPELL_STORM_UNLEASHED_ = 130908, ??
};

enum eEvents
{
    EVENT_ASSAULT               = 1,
    EVENT_SUMMON_TEMPEST        = 2,
    EVENT_STEP                  = 3,
    EVENT_PRE_UNSEEN_STRIKE     = 4,
    EVENT_UNSEEN_STRIKE         = 5,
    EVENT_BLADE_TEMPEST         = 6,
    EVENT_STORM_UNLEASHED_1     = 7,
    EVENT_STORM_UNLEASHED_2     = 8,
};

Position const windPos[25] =
{
    {-2082.48f, 336.129f, 420.47f},
    {-2097.69f, 336.248f, 421.48f},
    {-2142.02f, 258.427f, 422.24f},
    {-2094.6f,  198.141f, 422.24f},
    {-2082.71f, 227.596f, 420.47f},
    {-2170.01f, 225.606f, 420.47f},
    {-2141.73f, 280.427f, 420.98f},
    {-2095.72f, 371.101f, 422.24f},
    {-2068.11f, 226.205f, 420.47f},
    {-2141.84f, 302.484f, 422.24f},
    {-2158.48f, 281.554f, 420.98f},
    {-2065.23f, 337.177f, 420.47f},
    {-2095.64f, 225.509f, 420.98f},
    {-2141.29f, 227.524f, 421.48f},
    {-2096.57f, 301.944f, 422.24f},
    {-2058.79f, 281.871f, 425.16f},
    {-2096.56f, 281.399f, 420.98f},
    {-2155.09f, 227.217f, 420.46f},
    {-2096.02f, 258.227f, 422.24f},
    {-2142.29f, 196.33f,  422.24f},
    {-2076.68f, 282.052f, 422.56f},
    {-2153.78f, 337.132f, 420.44f},
    {-2142.38f, 336.856f, 421.48f},
    {-2141.77f, 369.038f, 422.24f},
    {-2169.5f,  336.578f, 420.47f}
};

class boss_lord_tayak : public CreatureScript
{
    public:
        boss_lord_tayak() : CreatureScript("boss_lord_tayak") {}

        struct boss_lord_tayakAI : public BossAI
        {
            boss_lord_tayakAI(Creature* creature) : BossAI(creature, DATA_LORD_TAYAK), summons(me) {}

            SummonList summons;
            ObjectGuid striketarget;
            bool LastPhaseWest, LastPhaseEast;

            void Reset()
            {
                _Reset();
                events.Reset();
                summons.DespawnAll();
                LastPhaseWest = false;
                LastPhaseEast = false;
                striketarget = ObjectGuid::Empty;
                me->SetReactState(REACT_DEFENSIVE);
                me->RemoveAurasDueToSpell(SPELL_INTENSIFY);
                me->RemoveAurasDueToSpell(SPELL_INTENSIFY_PHASE_TWO);
                me->RemoveAurasDueToSpell(SPELL_INTENSIFY_TRIGGER_EF);
                me->RemoveAurasDueToSpell(SPELL_STORM_UNLEASHED_VIS_1);
                instance->SetData(DATA_STORM_UNLEASHED, NOT_STARTED);
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                DoCast(me, SPELL_INTENSIFY, true);
                events.ScheduleEvent(EVENT_ASSAULT,           urand(15000, 20000));
                events.ScheduleEvent(EVENT_SUMMON_TEMPEST,    16000);
                events.ScheduleEvent(EVENT_STEP,              urand(25000, 35000));
                events.ScheduleEvent(EVENT_PRE_UNSEEN_STRIKE, 60000);
                if (IsHeroic())
                    events.ScheduleEvent(EVENT_BLADE_TEMPEST, 60000);
            }

            void WindStep()
            {
                DoStopAttack();
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 40.0f, true))
                {
                    float x, y, z;
                    target->GetPosition(x, y, z);
                    me->NearTeleportTo(x, y, z, 0.0f);

                    std::list<Player*> playerList;
                    GetPlayerListInGrid(playerList, me, 8.0f);
                    for (std::list<Player*>::iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
                    {
                        if (Player* pl = *itr)
                            pl->AddAura(SPELL_WIND_STEP, pl);
                    }
                }
                me->SetReactState(REACT_AGGRESSIVE);
                DoZoneInCombat(me, 100.0f);
                events.ScheduleEvent(EVENT_STEP, urand(25000, 35000));
            }

            void PrepareStrike()
            {
                striketarget = ObjectGuid::Empty;
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 40.0f, true))
                {
                    striketarget = target->GetGUID();
                    target->AddAura(SPELL_UNSEEN_STRIKE_TARGET, target);
                }

                if (striketarget)
                {
                    DoStopAttack();
                    me->SetVisible(false);
                    events.ScheduleEvent(EVENT_UNSEEN_STRIKE, 5000);
                }
            }

            void StartUnseenStrike()
            {
                if (Player* pl = me->GetPlayer(*me, striketarget))
                {
                    if (pl->isAlive())
                    {
                        float x, y, z;
                        pl->GetPosition(x, y, z);
                        me->NearTeleportTo(x, y, z, 0.0f);
                        me->SetVisible(true);
                        me->SetFacingToObject(pl);
                        DoCast(pl, SPELL_UNSEEN_STRIKE_DMG);
                    }
                }
                me->SetReactState(REACT_AGGRESSIVE);
                DoZoneInCombat(me, 100.0f);
                events.ScheduleEvent(EVENT_PRE_UNSEEN_STRIKE, 60000);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                summons.DespawnAll();
                instance->SetData(DATA_STORM_UNLEASHED, NOT_STARTED);
            }

            void JustSummoned(Creature* summoned)
            {
                summons.Summon(summoned);

                switch (summoned->GetEntry())
                {
                    case NPC_GALE_WINDS_STALKER:
                        summoned->SetReactState(REACT_PASSIVE);
                        summoned->CastSpell(summoned, 123633, true); // Gale Winds Visual Trigger
                        break;
                }
            }

            void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
            {
                if (HealthBelowPct(20) && !LastPhaseWest)
                {
                    LastPhaseWest = true;
                    events.CancelEvent(EVENT_STEP);
                    events.CancelEvent(EVENT_SUMMON_TEMPEST);
                    events.CancelEvent(EVENT_PRE_UNSEEN_STRIKE);
                    DoStopAttack();
                    summons.DespawnAll();
                    DoCast(me, SPELL_STORM_UNLEASHED_VIS_1, true);

                    me->RemoveAurasDueToSpell(SPELL_INTENSIFY);
                    me->GetMotionMaster()->MoveCharge(-2119.34f, 281.37f, 420.98f, 20.0f, 1); //Move to Centr
                }
                if (HealthBelowPct(10) && !LastPhaseEast)
                {
                    LastPhaseEast = true;
                    me->GetMotionMaster()->MoveCharge(-2119.15f, 184.44f, 422.24f, 30.0f, 3); //Move to East
                    summons.DespawnEntry(NPC_GALE_WINDS_STALKER);
                    for (uint8 i = 0; i < 25; i++)
                        if (Creature* sum = me->SummonCreature(NPC_GALE_WINDS_STALKER, windPos[i]))
                            sum->SetFacingTo(4.62f);
                }
            }

            void MovementInform(uint32 type, uint32 id)
            {
                if (type != POINT_MOTION_TYPE)
                    return;

                me->GetMotionMaster()->Clear(false);

                switch (id)
                {
                    case 1:
                        events.ScheduleEvent(EVENT_STORM_UNLEASHED_1, 1000);
                        if (instance->GetData(DATA_STORM_UNLEASHED) != IN_PROGRESS)
                            instance->SetData(DATA_STORM_UNLEASHED, IN_PROGRESS);
                        break;
                    case 2:
                        DoCast(me, SPELL_STORM_UNLEASHED_AURA, true);
                        DoCast(me, SPELL_INTENSIFY_PHASE_TWO, true);
                        break;
                    case 3:
                        DoCast(me, SPELL_STORM_UNLEASHED_AURA, true);
                        break;
                }
            }

            void SpellHitTarget(Unit* target, const SpellInfo* spell)
            {
                if (target->GetTypeId() != TYPEID_UNIT || spell->Id != 123616)
                    return;

                switch (target->GetEntry())
                {
                    case NPC_STORM_EAST_1_TARGET:
                        target->CastSpell(target, 123643);
                        break;
                    case NPC_STORM_EAST_2_TARGET:
                        target->CastSpell(target, 123644);
                        break;
                    case NPC_STORM_EAST_3_TARGET:
                        target->CastSpell(target, 123645);
                        break;
                    case NPC_STORM_WEST_1_TARGET:
                        target->CastSpell(target, 123597);
                        break;
                    case NPC_STORM_WEST_2_TARGET:
                        target->CastSpell(target, 123639);
                        break;
                    case NPC_STORM_WEST_3_TARGET:
                        target->CastSpell(target, 123640);
                        break;
                }
            }

            void UpdateAI(uint32 diff)
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
                        case EVENT_ASSAULT:
                            if (me->getVictim())
                                DoCast(me->getVictim(), SPELL_OVERWHELMING_ASSAULT);
                            events.ScheduleEvent(EVENT_ASSAULT, urand(15000, 20000));
                            break;
                        case EVENT_SUMMON_TEMPEST:
                            DoCast(SPELL_TEMPEST_SLASH_SUM);
                            events.ScheduleEvent(EVENT_SUMMON_TEMPEST, 16000);
                            break;
                        case EVENT_STEP:
                            events.DelayEvents(1500);
                            WindStep();
                            break;
                        case EVENT_PRE_UNSEEN_STRIKE:
                            events.DelayEvents(7000);
                            PrepareStrike();
                            break;
                        case EVENT_UNSEEN_STRIKE:
                            StartUnseenStrike();
                            break;
                        case EVENT_BLADE_TEMPEST:
                            DoCast(me, SPELL_BLADE_TEMPEST_JUMP, true);
                            DoCast(SPELL_BLADE_TEMPEST);
                            events.ScheduleEvent(EVENT_BLADE_TEMPEST, 60000);
                            break;
                        case EVENT_STORM_UNLEASHED_1:
                            DoCast(me, SPELL_STORM_UNLEASHED_DMG, true);
                            DoCast(me, SPELL_STORM_UNLEASHED_JUMP, true);
                            events.ScheduleEvent(EVENT_STORM_UNLEASHED_2, 2000);
                            break;
                        case EVENT_STORM_UNLEASHED_2:
                        {
                            me->GetMotionMaster()->MoveCharge(-2118.69f, 378.26f, 422.24f, 30.0f, 2); //Move to West
                            for (uint8 i = 0; i < 25; i++)
                                if (Creature* sum = me->SummonCreature(NPC_GALE_WINDS_STALKER, windPos[i]))
                                    sum->SetFacingTo(1.53f);
                            break;
                        }
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_lord_tayakAI(creature);
        }
};

//62908
class npc_tempest_slash : public CreatureScript
{
    public:
        npc_tempest_slash() : CreatureScript("npc_tempest_slash") {}

        struct npc_tempest_slashAI : public ScriptedAI
        {
            npc_tempest_slashAI(Creature* creature) : ScriptedAI(creature)
            {
                InstanceScript* pInstance = creature->GetInstanceScript();
                me->SetDisplayId(11686);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
            }

            void Reset()
            {
                events.Reset();
            }

            void MovementInform(uint32 type, uint32 id)
            {
                if (type == POINT_MOTION_TYPE)
                    events.ScheduleEvent(EVENT_1, 1000);
            }

            void IsSummonedBy(Unit* summoner)
            {
                DoCast(me, SPELL_TEMPEST_SLASH_DMG, true);
                if (summoner->ToCreature())
                    if (Unit* target = summoner->ToCreature()->AI()->SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true))
                    {
                        Position pos;
                        target->GetPosition(&pos);
                        me->GetMotionMaster()->MoveCharge(pos.GetPositionX(), pos.GetPositionY(), me->GetPositionZ(), 20.0f);
                    }
                    else if (!target)
                        events.ScheduleEvent(EVENT_1, 1000);
            }

            void FillCirclePath(Position const& centerPos, float radius, float z, Movement::PointsArray& path)
            {
                float stepbyangle = 2*M_PI / 8;

                for (uint8 i = 0; i < 8; ++i)
                {
                    G3D::Vector3 point;
                    point.x = centerPos.GetPositionX() + radius * std::cos(stepbyangle*i);
                    point.y = centerPos.GetPositionY() + radius * std::sin(stepbyangle*i);
                    z = me->GetMap()->GetHeight(point.x, point.y, z);
                    point.z = z;
                    path.push_back(point);
                }
            }

            void UpdateAI(uint32 diff)
            {
                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_1:
                        {
                            Position pos;
                            me->GetPosition(&pos);
                            me->SetSpeed(MOVE_RUN, 4.0f, true);
                            Movement::MoveSplineInit init(*me);
                            FillCirclePath(pos, 5.0f, me->GetPositionZ(), init.Path());
                            init.SetCyclic();
                            init.SetWalk(false);
                            init.SetVelocity(4.0f);
                            init.Launch();
                            break;
                        }
                    }
                }
            }
        private:
            InstanceScript* pInstance;
            EventMap events;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_tempest_slashAI(creature);
        }
};

//63567
class npc_tayak_storm_unleashed_plr_veh : public CreatureScript
{
    public:
        npc_tayak_storm_unleashed_plr_veh() : CreatureScript("npc_tayak_storm_unleashed_plr_veh") {}

        struct npc_tayak_storm_unleashed_plr_vehAI : public ScriptedAI
        {
            npc_tayak_storm_unleashed_plr_vehAI(Creature* creature) : ScriptedAI(creature)
            {
                InstanceScript* pInstance = creature->GetInstanceScript();
            }

            InstanceScript* pInstance;

            void Reset() {}

            void MovementInform(uint32 type, uint32 id)
            {
                if (type != POINT_MOTION_TYPE)
                    return;

                if (me->GetVehicleKit())
                    me->GetVehicleKit()->RemoveAllPassengers();

                me->DespawnOrUnsummon();
            }

            void IsSummonedBy(Unit* summoner)
            {
                DoCast(me, SPELL_STORM_UNLEASHED_VIS_2, true);
                summoner->CastSpell(me, SPELL_RIDE_VEHICLE, true);
            }

            void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
            {
                if (!apply)
                    return;

                me->GetMotionMaster()->MovePoint(1, -2119.15f, 184.44f, 422.24f);
            }

            void UpdateAI(uint32 diff) {}
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_tayak_storm_unleashed_plr_vehAI(creature);
        }
};


// WEST - 63278, 63299, 63300
// EAST - 63301, 63302, 63303
class npc_tayak_storm_unleashed_veh : public CreatureScript
{
    public:
        npc_tayak_storm_unleashed_veh() : CreatureScript("npc_tayak_storm_unleashed_veh") {}

        struct npc_tayak_storm_unleashed_vehAI : public ScriptedAI
        {
            npc_tayak_storm_unleashed_vehAI(Creature* creature) : ScriptedAI(creature)
            {
                InstanceScript* pInstance = creature->GetInstanceScript();
            }

            InstanceScript* pInstance;
            EventMap events;

            void Reset() {}

            void MovementInform(uint32 type, uint32 id)
            {
                if (type != POINT_MOTION_TYPE)
                    return;

                if (me->GetVehicleKit())
                    me->GetVehicleKit()->RemoveAllPassengers();

                me->DespawnOrUnsummon();
            }

            void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
            {
                if (apply)
                    me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_CONTROL_VEHICLE, true);
                else
                    events.ScheduleEvent(EVENT_1, 2000);
            }

            void IsSummonedBy(Unit* summoner)
            {
                DoCast(me, SPELL_RIDE_VEHICLE_2, true);

                switch (me->GetEntry())
                {
                    case NPC_STORM_WEST_1_SUM:
                        me->GetMotionMaster()->MovePoint(1, -2126.17f, 180.73f, 422.24f);
                        break;
                    case NPC_STORM_WEST_2_SUM:
                        me->GetMotionMaster()->MovePoint(1, -2118.94f, 180.39f, 422.24f);
                        break;
                    case NPC_STORM_WEST_3_SUM:
                        me->GetMotionMaster()->MovePoint(1, -2111.47f, 180.90f, 422.24f);
                        break;
                    case NPC_STORM_EAST_1_SUM:
                        me->GetMotionMaster()->MovePoint(1, -2126.27f, 384.36f, 422.24f);
                        break;
                    case NPC_STORM_EAST_2_SUM:
                        me->GetMotionMaster()->MovePoint(1, -2118.22f, 383.89f, 422.24f);
                        break;
                    case NPC_STORM_EAST_3_SUM:
                        me->GetMotionMaster()->MovePoint(1, -2110.92f, 384.06f, 422.24f);
                        break;
                }
            }

            void UpdateAI(uint32 diff)
            {
                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_1:
                            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_CONTROL_VEHICLE, false);
                            break;
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_tayak_storm_unleashed_vehAI(creature);
        }
};

//8196 8197 - 126159
class at_storm_unleashed : public AreaTriggerScript
{
    public:

        at_storm_unleashed() : AreaTriggerScript("at_storm_unleashed") {}

        bool OnTrigger(Player* player, AreaTriggerEntry const* /*trigger*/, bool apply)
        {
            if (InstanceScript* pInstance = player->GetInstanceScript())
            {
                if (pInstance->GetData(DATA_STORM_UNLEASHED) != IN_PROGRESS)
                    return false;

                if (apply)
                    player->CastSpell(player, 126159, true);
                else
                    player->RemoveAurasDueToSpell(126159);
            }

            return false;
        }
};

void AddSC_boss_lord_tayak()
{
    new boss_lord_tayak();
    new npc_tempest_slash();
    new npc_tayak_storm_unleashed_plr_veh();
    new npc_tayak_storm_unleashed_veh();
    new at_storm_unleashed();
}
