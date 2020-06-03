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

#include "siege_of_orgrimmar.h"

enum eSpells
{ 
    //Phase 1
    SPELL_FLAME_VENTS             = 144464,
    SPELL_MORTAR_BLAST            = 146027,
    SPELL_SCATTER_LASER           = 144458,
    SPELL_DETONATION_SEQUENCE     = 144718,
    SPELL_CRAWLER_MINE_BLAST      = 144766,
    SPELL_BORER_DRILL_VISUAL      = 144221,
    SPELL_GROUND_POUND            = 144776,
    SPELL_ENGULFED_EXPLOSE        = 144791,
    SPELL_DEMOLISHER_CANNON       = 144153,
    SPELL_BORER_DRILL_TR_VISUAL   = 144221,
    SPELL_BORER_DRILL_B_VISUAL    = 144296,
    SPELL_BORER_DRILL_DMG         = 144218,
    //HM
    SPELL_MORTAR_BARRAGE          = 144553,
    SPELL_MORTAR_BARRAGE_PERIODIC = 144554,
    SPELL_RICOCHET_TR_VISUAL      = 144375,
    SPELL_RICOCHET_DMG            = 144327,
    SPELL_RICOCHET_AT             = 144356,
    //Phase 2
    SPELL_SEISMIC_ACTIVITY        = 144483,
    SPELL_SEISMIC_ACTIVITY_VISUAL = 144557,
    SPELL_SHOCK_PULSE             = 144485,
    SPELL_CUTTER_LASER_VISUAL     = 144576,
    SPELL_CUTTER_LASER_DMG        = 144918,
    SPELL_CUTTER_LASER_TARGET_V   = 146325,
    SPELL_EXPLOSIVE_TAR_SUMMON    = 144496,
    SPELL_EXPLOSIVE_TAR_DMG       = 144498,
    SPELL_EXPLOSIVE_TAR_VISUAL    = 146191,
    SPELL_EXPLOSIVE_TAR_AT        = 144525,
    SPELL_TAR_EXPLOSION           = 144919,
    SPELL_BERSERK                 = 26662,
    SPELL_BERSERK2                = 64112,
};

enum eEvents
{
    //iron juggernat
    EVENT_MORTAR_BLAST            = 1,
    EVENT_FLAME_VENTS             = 2,
    EVENT_SCATTER_LASER           = 3,
    EVENT_SUMMON_MINE             = 4,
    EVENT_CUTTER_LASER            = 5,
    EVENT_EXPLOSIVE_TAR           = 6,
    EVENT_SHOCK_PULSE             = 7,
    EVENT_DEMOLISHER_CANNON       = 8,
    EVENT_BORER_DRILL             = 9,
    EVENT_MORTAR_BARRAGE          = 10,
    //Mines
    EVENT_ACTIVE_DETONATE         = 11,
    EVENT_ENGULFED_EXPLOSE        = 12,
    //Cutter Laser
    EVENT_ACTIVE_EXPLOSIVE_TAR    = 13,
    //Special
    EVENT_CHECK_PROGRESS          = 14,
};

Position const modpos[3] = 
{
    {0.0f,  0.0f,  0.0f},
    {14.0f, 4.0f,  0.0f},
    {4.0f, -14.0f, 0.0f},
};

enum Phases
{
    PHASE_ONE                     = 1,
    PHASE_TWO                     = 2,
};

enum Actions
{
    ACTION_PHASE_ONE              = 1,
    ACTION_PHASE_TWO              = 2,
};

//71466
class boss_iron_juggernaut : public CreatureScript
{
    public:
        boss_iron_juggernaut() : CreatureScript("boss_iron_juggernaut") {}

        struct boss_iron_juggernautAI : public BossAI
        {
            boss_iron_juggernautAI(Creature* creature) : BossAI(creature, DATA_IRON_JUGGERNAUT)
            {
                instance = creature->GetInstanceScript();
            }

            InstanceScript* instance;
            uint32 PowerTimer, enrage, checkvictim;
            Phases phase;

            void Reset()
            {
                _Reset();
                checkvictim = 0;
                PowerTimer = 0;
                phase = PHASE_ONE;
                events.SetPhase(PHASE_ONE);
                me->SetReactState(REACT_DEFENSIVE);
                me->setPowerType(POWER_ENERGY);
                me->SetPower(POWER_ENERGY, 0);
                me->RemoveAurasDueToSpell(SPELL_SEISMIC_ACTIVITY);
                me->RemoveAurasDueToSpell(SPELL_BERSERK);
                me->RemoveAurasDueToSpell(SPELL_BERSERK2);
                SendActionForAllPassenger(false);
                if (instance)
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BORER_DRILL_DMG);
            }

            void JustReachedHome()
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                me->SetReactState(REACT_DEFENSIVE);
            }

            bool CheckPullPlayerPos(Unit* who)
            {
                if (who->GetPositionX() < 1258.00f)
                    return false;
                return true;
            }

            void EnterCombat(Unit* who)
            {
                events.Reset();
                _EnterCombat();
                SendActionForAllPassenger(true);
                PowerTimer = 1100;
                checkvictim = 1500;
                enrage = 600000;
                events.RescheduleEvent(EVENT_CHECK_PROGRESS, 5000);
                events.RescheduleEvent(EVENT_SUMMON_MINE, 30000);
                events.RescheduleEvent(EVENT_DEMOLISHER_CANNON, 9000);
                events.RescheduleEvent(EVENT_BORER_DRILL, 20000, 0, PHASE_ONE);
                events.RescheduleEvent(EVENT_SCATTER_LASER, 11500, 0, PHASE_ONE);
                events.RescheduleEvent(EVENT_FLAME_VENTS, 7000, 0, PHASE_ONE);
                events.RescheduleEvent(EVENT_MORTAR_BLAST, 30000, 0, PHASE_ONE);
            }

            void EnterEvadeMode()
            {
                me->NearTeleportTo(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY(), me->GetHomePosition().GetPositionZ(), me->GetHomePosition().GetOrientation());
                ScriptedAI::EnterEvadeMode();
            }

            void DoAction(int32 const action)
            {
                switch (action)
                {
                case ACTION_PHASE_ONE:
                    PowerTimer = 0;
                    events.Reset();
                    phase = PHASE_ONE;
                    events.SetPhase(PHASE_ONE);
                    me->SetPower(POWER_ENERGY, 0);
                    me->SetReactState(REACT_AGGRESSIVE);
                    PowerTimer = 1100;
                    events.RescheduleEvent(EVENT_SUMMON_MINE, 30000);
                    events.RescheduleEvent(EVENT_DEMOLISHER_CANNON, 9000);
                    events.RescheduleEvent(EVENT_BORER_DRILL, 20000, 0, PHASE_ONE);
                    events.RescheduleEvent(EVENT_SCATTER_LASER, 11500, 0, PHASE_ONE);
                    events.RescheduleEvent(EVENT_FLAME_VENTS, 12000, 0, PHASE_ONE);
                    events.RescheduleEvent(EVENT_MORTAR_BLAST, 30000, 0, PHASE_ONE);
                    break;
                case ACTION_PHASE_TWO:
                    events.Reset();
                    me->StopAttack();
                    PowerTimer = 600;
                    DoCast(me, SPELL_SEISMIC_ACTIVITY_VISUAL, true);
                    DoCast(me, SPELL_SEISMIC_ACTIVITY);
                    events.RescheduleEvent(EVENT_SUMMON_MINE, 30000);
                    events.RescheduleEvent(EVENT_DEMOLISHER_CANNON, 9000);
                    events.RescheduleEvent(EVENT_EXPLOSIVE_TAR, 10000, 0, PHASE_TWO);
                    events.RescheduleEvent(EVENT_SHOCK_PULSE, 16500, 0, PHASE_TWO);
                    break;
                }
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (PowerTimer)
                {
                    if (PowerTimer <= diff)
                    {
                        switch (phase)
                        {
                        case PHASE_ONE:
                            if (me->GetPower(POWER_ENERGY) <= 99)
                                me->SetPower(POWER_ENERGY, me->GetPower(POWER_ENERGY) + 1, true);

                            if (me->GetPower(POWER_ENERGY) == 100)
                            {
                                phase = PHASE_TWO;
                                PowerTimer = 0;
                                events.SetPhase(PHASE_TWO);
                                DoAction(ACTION_PHASE_TWO);
                                return;
                            }
                            PowerTimer = 1200;
                            break;
                        case PHASE_TWO:
                            if (me->GetPower(POWER_ENERGY) >= 1)
                            {
                                me->SetPower(POWER_ENERGY, me->GetPower(POWER_ENERGY) - 1, true);
                                PowerTimer = 600;
                            }
                            break;
                        default:
                            break;
                        }
                    }
                    else
                        PowerTimer -= diff;
                }

                if (checkvictim && instance)
                {
                    if (checkvictim <= diff)
                    {
                        if (me->getVictim())
                        {
                            if (!CheckPullPlayerPos(me->getVictim()))
                            {
                                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                                EnterEvadeMode();
                                checkvictim = 0;
                            }
                            else
                                checkvictim = 1500;
                        }
                    }
                    else
                        checkvictim -= diff;
                }

                if (enrage)
                {
                    if (enrage <= diff)
                    {
                        enrage = 0;
                        DoCast(me, SPELL_BERSERK, true);
                    }
                    else
                        enrage -= diff;
                }

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_CHECK_PROGRESS:
                        if (instance && instance->GetBossState(DATA_GALAKRAS) != DONE)
                        {
                            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                            EnterEvadeMode();
                        }
                        break;
                    case EVENT_BORER_DRILL:
                    {
                        DoCast(me, SPELL_BORER_DRILL_B_VISUAL, true);
                        Position pos;
                        me->GetNearPosition(pos, 12.5f, 5.32f);
                        std::list<Player*> pllist;
                        pllist.clear();
                        GetPlayerListInGrid(pllist, me, 100.0f);
                        uint8 count = 0;
                        if (!pllist.empty())
                        {
                            for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                            {
                                if (!(*itr)->isInTankSpec()) 
                                {
                                    if (Creature* drill = me->SummonCreature(NPC_BORER_DRILL, pos, TEMPSUMMON_TIMED_DESPAWN, 10000))
                                    {
                                        count++;
                                        drill->AddThreat((*itr), 50000000.0f);
                                        drill->SetReactState(REACT_AGGRESSIVE);
                                        drill->Attack((*itr), true); 
                                        drill->GetMotionMaster()->MoveChase(*itr);
                                        if (count >= 3)
                                            break;
                                    }
                                }
                            }
                        }
                        events.RescheduleEvent(EVENT_BORER_DRILL, 20000, 0, PHASE_ONE);
                        break;
                    }
                    case EVENT_MORTAR_BLAST:
                        if (Unit* p = GetPassengerForCast(NPC_TOP_CANNON))
                        {
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 80.0f, true))
                            {
                                p->SetFacingToObject(target);
                                p->CastSpell(target, SPELL_MORTAR_BLAST);
                            }
                        }
                        events.RescheduleEvent(EVENT_MORTAR_BLAST, 30000, 0, PHASE_ONE);
                        break;
                    case EVENT_DEMOLISHER_CANNON:
                        if (Unit* p = GetPassengerForCast(NPC_CANNON))
                        {
                            uint8 num = 0;
                            std::list<Player*>pllist;
                            pllist.clear();
                            GetPlayerListInGrid(pllist, me, 80.0f);
                            if (!pllist.empty())
                            {
                                for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                                {
                                    if (!(*itr)->isInTankSpec())
                                    {
                                        p->CastSpell(*itr, SPELL_DEMOLISHER_CANNON);
                                        num++;
                                        if (num == 3)
                                            break;
                                    }
                                }
                            }
                        }
                        events.RescheduleEvent(EVENT_DEMOLISHER_CANNON, 9000);
                        break;
                    case EVENT_SCATTER_LASER:
                        if (Unit* p = GetPassengerForCast(NPC_TAIL_GUN))
                        {
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 80.0f, true)) 
                            {
                                p->SetFacingToObject(target);
                                p->CastSpell(target, SPELL_SCATTER_LASER);
                            }
                        }
                        events.RescheduleEvent(EVENT_SCATTER_LASER, 11500, 0, PHASE_ONE);
                        break;
                    case EVENT_SUMMON_MINE:
                        if (Unit* target = SelectTarget(SELECT_TARGET_FARTHEST, 1, 80.0f, true))
                        {
                            float x, y, z;
                            x = target->GetPositionX();
                            y = target->GetPositionY();
                            z = target->GetPositionZ();
                            for (uint8 n = 0; n < 3; n++)
                                if (Creature* mine = me->SummonCreature(NPC_CRAWLER_MINE, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ()))
                                    mine->GetMotionMaster()->MoveCharge(x + modpos[n].GetPositionX(), y + modpos[n].GetPositionY(), z, 20.0f, 0);
                        }
                        events.RescheduleEvent(EVENT_SUMMON_MINE, 30000);
                        break;
                    case EVENT_EXPLOSIVE_TAR:
                    {
                        float x, y;
                        for (uint8 n = 0; n < 5; n++)
                        {
                            GetPosInRadiusWithRandomOrientation(me, float(urand(20, 40)), x, y);
                            me->CastSpell(x, y, me->GetPositionZ(), SPELL_EXPLOSIVE_TAR_SUMMON);
                        }
                        events.RescheduleEvent(EVENT_CUTTER_LASER, 10000, 0, PHASE_TWO);
                        break;
                    }
                    case EVENT_CUTTER_LASER:                  
                        if (Unit* target = SelectTarget(SELECT_TARGET_FARTHEST, 0, 150.0f, true))
                        {
                            if (Creature* laser = me->SummonCreature(NPC_CUTTER_LASER, target->GetPositionX() + 10.0f, target->GetPositionY(), target->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 31000))
                            {
                                if (Unit* p = GetPassengerForCast(NPC_TAIL_GUN))
                                {
                                    p->CastSpell(laser, SPELL_CUTTER_LASER_VISUAL);
                                    laser->AddAura(SPELL_CUTTER_LASER_TARGET_V, target);
                                    laser->AddThreat(target, 50000000.0f);
                                    laser->SetReactState(REACT_AGGRESSIVE);
                                    laser->Attack(target, true);
                                    laser->GetMotionMaster()->MoveChase(target);
                                }
                            }
                        }
                        events.RescheduleEvent(EVENT_EXPLOSIVE_TAR, 15000, 0, PHASE_TWO);
                        break;
                    case EVENT_FLAME_VENTS:
                        if (me->getVictim())
                            DoCastVictim(SPELL_FLAME_VENTS, true);
                        events.RescheduleEvent(EVENT_FLAME_VENTS, 12000, 0, PHASE_ONE);
                        break;
                    case EVENT_SHOCK_PULSE:
                        DoCast(me, SPELL_SHOCK_PULSE);
                        if (me->GetMap()->IsHeroic())
                            events.RescheduleEvent(EVENT_MORTAR_BARRAGE, 4000);
                        events.RescheduleEvent(EVENT_SHOCK_PULSE, 16500, 0, PHASE_TWO);
                        break;
                    case EVENT_MORTAR_BARRAGE:
                        DoCast(me, SPELL_MORTAR_BARRAGE_PERIODIC, true);
                        break;
                    default:
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BORER_DRILL_DMG);
            }

            void SendActionForAllPassenger(bool action)
            {
                if (Vehicle* ij = me->GetVehicleKit())
                {
                    if (action)
                    {
                        for (uint8 n = 0; n < 5; n++)
                            if (Unit* p = ij->GetPassenger(n))
                                if (p->ToCreature() && p->ToCreature()->AI())
                                    p->ToCreature()->AI()->DoZoneInCombat(p->ToCreature(), 158.0f);
                    }
                    else
                    {
                        for (uint8 n = 0; n < 5; n++)
                            if (Unit* p = ij->GetPassenger(n))
                                p->ToCreature()->AI()->EnterEvadeMode();
                    }
                }
            }

            Unit* GetPassengerForCast(uint32 entry)
            {
                if (Vehicle* ij = me->GetVehicleKit())
                {
                    uint8 n;
                    switch (entry)
                    {
                    case NPC_TOP_CANNON:
                        n = 0;
                        break;
                    case NPC_SAWBLADE:
                        n = 1;
                        break;
                    case NPC_CANNON:
                        n = urand(2, 3);
                        break;
                    case NPC_TAIL_GUN:
                        n = 4;
                        break;
                    default:
                        return NULL;
                    }

                    if (Unit* p = ij->GetPassenger(n))
                        return p;
                }
                return NULL;
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_iron_juggernautAI(creature);
        }
};

//71484(0), 71469(1), 71468(2, 3), 71914(4) - (seatId);
class npc_generic_iron_juggernaut_passenger : public CreatureScript
{
public:
    npc_generic_iron_juggernaut_passenger() : CreatureScript("npc_generic_iron_juggernaut_passenger") {}

    struct npc_generic_iron_juggernaut_passengerAI : public ScriptedAI
    {
        npc_generic_iron_juggernaut_passengerAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        }

        InstanceScript* instance;

        void Reset()
        {
            me->SetReactState(REACT_PASSIVE);
        }
        
        void EnterCombat(Unit* who){}
        
        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_generic_iron_juggernaut_passengerAI(creature);
    }
};

//72050
class npc_crawler_mine : public CreatureScript
{
public:
    npc_crawler_mine() : CreatureScript("npc_crawler_mine") {}

    struct npc_crawler_mineAI : public ScriptedAI
    {
        npc_crawler_mineAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->setFaction(35);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
        }

        InstanceScript* instance;
        EventMap events;
        ObjectGuid targetGuid;

        void Reset()
        {
            events.Reset();
            targetGuid.Clear();
            events.RescheduleEvent(EVENT_ACTIVE_DETONATE, urand(3000, 5000));
        }

        void OnSpellClick(Unit* clicker)
        {
            if (clicker->ToPlayer() && me->HasAura(SPELL_DETONATION_SEQUENCE))
            {
                me->RemoveAurasDueToSpell(SPELL_DETONATION_SEQUENCE);
                targetGuid = clicker->GetGUID();
                clicker->CastSpell(me, SPELL_GROUND_POUND, true);
                events.RescheduleEvent(EVENT_ENGULFED_EXPLOSE, 1250);
            }
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            damage = 0;
        }
        
        void MovementInform(uint32 type, uint32 pointId)
        {
            if (type == POINT_MOTION_TYPE)
                if (pointId == 0)
                    events.RescheduleEvent(EVENT_ACTIVE_DETONATE, urand(3000, 5000));
        }

        void SpellHit(Unit* caster, SpellInfo const *spell)
        {   //after detonate
            if (spell->Id == SPELL_CRAWLER_MINE_BLAST)
            {
                me->Kill(me, true);
                me->DespawnOrUnsummon();
            }
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_ACTIVE_DETONATE:
                    DoCast(me, SPELL_DETONATION_SEQUENCE, true);
                    break;
                case EVENT_ENGULFED_EXPLOSE:
                    if (Player *pl = me->GetPlayer(*me, targetGuid))
                        pl->CastSpell(pl, SPELL_ENGULFED_EXPLOSE);
                    me->DespawnOrUnsummon();
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_crawler_mineAI(creature);
    }
};

//72026
class npc_cutter_laser : public CreatureScript
{
public:
    npc_cutter_laser() : CreatureScript("npc_cutter_laser") {}

    struct npc_cutter_laserAI : public ScriptedAI
    {
        npc_cutter_laserAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetFloatValue(OBJECT_FIELD_SCALE, 1.5f);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        }

        InstanceScript* instance;

        void Reset(){}         

        void EnterCombat(Unit* who){}
        
        void EnterEvadeMode(){}
        
        void UpdateAI(uint32 diff){}     
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_cutter_laserAI(creature);
    }
};

//71950
class npc_explosive_tar : public CreatureScript
{
public:
    npc_explosive_tar() : CreatureScript("npc_explosive_tar") {}

    struct npc_explosive_tarAI : public ScriptedAI
    {
        npc_explosive_tarAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        }
        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            events.RescheduleEvent(EVENT_ACTIVE_EXPLOSIVE_TAR, 2000);
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_ACTIVE_EXPLOSIVE_TAR)
                {
                    DoCast(me, SPELL_EXPLOSIVE_TAR_AT, true);
                    DoCast(me, SPELL_EXPLOSIVE_TAR_VISUAL);
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_explosive_tarAI(creature);
    }
};

//71906
class npc_borer_drill : public CreatureScript
{
public:
    npc_borer_drill() : CreatureScript("npc_borer_drill") {}

    struct npc_borer_drillAI : public ScriptedAI
    {
        npc_borer_drillAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetFloatValue(OBJECT_FIELD_SCALE, 1.5f);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            DoCast(me, SPELL_BORER_DRILL_TR_VISUAL, true);
        }

        InstanceScript* instance;

        void Reset(){}

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_borer_drillAI(creature);
    }
};

//90002
class npc_mortar_barrage : public CreatureScript
{
public:
    npc_mortar_barrage() : CreatureScript("npc_mortar_barrage") {}

    struct npc_mortar_barrageAI : public ScriptedAI
    {
        npc_mortar_barrageAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
        }

        InstanceScript* instance;

        void Reset(){}

        void EnterCombat(Unit* who){}

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            damage = 0;
        }

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_mortar_barrageAI(creature);
    }
};

//146325
class spell_cutter_laser_target : public SpellScriptLoader
{
public:
    spell_cutter_laser_target() : SpellScriptLoader("spell_cutter_laser_target") { }

    class spell_cutter_laser_target_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_cutter_laser_target_AuraScript);

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*handle*/)
        {
            if (GetCaster() && GetCaster()->ToCreature())
            {
                GetCaster()->RemoveAurasDueToSpell(SPELL_CUTTER_LASER_VISUAL);
                GetCaster()->ToCreature()->DespawnOrUnsummon();
            }         
        }
        
        void Register()
        {
            AfterEffectRemove += AuraEffectRemoveFn(spell_cutter_laser_target_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_FIXATE, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_cutter_laser_target_AuraScript();
    }
};

//144483
class spell_seismic_activity : public SpellScriptLoader
{
public:
    spell_seismic_activity() : SpellScriptLoader("spell_seismic_activity") { }

    class spell_seismic_activity_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_seismic_activity_AuraScript);

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*handle*/)
        {
            if (GetCaster() && GetCaster()->ToCreature())
                GetCaster()->ToCreature()->AI()->DoAction(ACTION_PHASE_ONE);
        }

        void Register()
        {
            AfterEffectRemove += AuraEffectRemoveFn(spell_seismic_activity_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_seismic_activity_AuraScript();
    }
};

//144554
class spell_mortar_barrage : public SpellScriptLoader
{
public:
    spell_mortar_barrage() : SpellScriptLoader("spell_mortar_barrage") { }

    class spell_mortar_barrage_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_mortar_barrage_AuraScript);

        void OnTick(AuraEffect const* aurEff)
        {
            if (GetCaster())
            {
                float x, y;
                uint8 val = urand(3, 4);
                for (uint8 n = 0; n < val; n++)
                {
                    GetPosInRadiusWithRandomOrientation(GetCaster(), float(urand(20, 60)), x, y);
                    if (Creature* mb = GetCaster()->SummonCreature(NPC_MORTAR_BARRAGE, x, y, GetCaster()->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 4000))
                        GetCaster()->CastSpell(mb, SPELL_MORTAR_BLAST, true);
                }
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_mortar_barrage_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_mortar_barrage_AuraScript();
    }
};

//144919
class spell_explosive_tar : public SpellScriptLoader
{
public:
    spell_explosive_tar() : SpellScriptLoader("spell_explosive_tar") { }

    class spell_explosive_tar_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_explosive_tar_SpellScript);

        void HandleAfterCast()
        {
            if (GetSpellInfo()->Id == SPELL_TAR_EXPLOSION)
                if (GetCaster() && GetCaster()->ToCreature())
                        GetCaster()->ToCreature()->DespawnOrUnsummon();
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_explosive_tar_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_explosive_tar_SpellScript();
    }
};

void AddSC_boss_iron_juggernaut()
{
    new boss_iron_juggernaut();
    new npc_generic_iron_juggernaut_passenger();
    new npc_crawler_mine();
    new npc_cutter_laser();
    new npc_explosive_tar();
    new npc_borer_drill();
    //new npc_mortar_barrage();
    new spell_cutter_laser_target();
    new spell_seismic_activity();
    new spell_mortar_barrage();
    new spell_explosive_tar();
}
