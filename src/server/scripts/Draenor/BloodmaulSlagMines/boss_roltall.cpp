/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
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

#include "bloodmaul_slag_mines.h"
#include "MoveSplineInit.h"

enum Spells
{
    //< shards
    SPELL_FIERY_BOULDER         = 153247,
    //SPELL_FIERY_BOULDER_1       = 152741, // not exist at sniffs from 6.1.2 ?
    SPELL_FIERY_BOULDER_2       = 152850, // casted by NPC_FIERY_BOULDER_XXXX to self
    SPELL_FIERY_BOULDER_3       = 153058, // casted by NPC_FIERY_BOULDER_XXXX to self
    SPELL_FIERY_BOULDER_4       = 152742, // AT casted summon to boulder ground pos
    SPELL_FIERY_BOULDER_5       = 152835, // AT casted by NPC_FIERY_BOULDER_XXX to self with DstLocation
    SPELL_FIERY_BOULDER_6       = 152837, // AT casted by NPC_FIERY_BOULDER_MIDDLE to self with DstLocation
    SPELL_FIERY_BOULDER_7       = 152843, // casted at reach any players in radius
    SPELL_FIERY_BOULDER_8       = 163733, // casted by NPC_FIERY_BOULDER_XXXX to self
    SPELL_ALPHA_FADE_OUT        = 141608,

    SPELL_BURNING_SLAG          = 152918,
    SPELL_BURNING_SLAG_M_TRIGGER= 152913,
    SPELL_BURNING_SLAG_         = 153227,
    SPELL_BURNING_SLAG_2        = 152939,

    SPELL_HEAT_WAVE             = 152897,
    SPELL_HEAT_WAVE_2           = 152940,
    SPELL_HEAT_WAVE_3           = 152896,
    SPELL_HEAT_WAVE_4           = 152867,

    SPELL_SCORCHING_AURA        = 167739,
    SPELL_SCORCHING_AURA_AT     = 167738,
};

enum NPCs
{
    NPC_HEAT_WAVE               = 75865, //< casting SPELL_HEAT_WAVE

    NPC_FIERY_BOULDER_RIGHT     = 75828,
    NPC_FIERY_BOULDER_LEFT      = 75853,
    NPC_FIERY_BOULDER_MIDDLE    = 75854,
};

Position const rightBoulder[] =   //< NPC_FIERY_BOULDER_RIGHT
{
    {2297.642f, -204.163f, 253.099f},   // стартовая позиция Falling (64) 
    {2297.642f, -204.163f, 212.631f},   // позиция 2 - упали в низ, как только достигаем позиции вешаем Flags: Flying (512) 
    {2296.247f, -204.237f, 213.545f},   // [0] WayPoints точка старта от босса
    {2249.747f, -205.987f, 213.545f}    // [1] WayPoints точка на другой стороне моста

    /*
    Кастуем сами на себя спелл SPELL_FIERY_BOULDER_1 и получаем триггером SPELL_FIERY_BOULDER_2

    Стартовая позиция:
    Position:       2297.642 Y: -204.163, 253.099     Flags: Falling (64)

    прямо от неё мы двигаемся сюда
    Position:       2297.642 Y: -204.163, 212.631

    через 2 секунды кастуем на себя SPELL_FIERY_BOULDER_3 и получаем триггером SPELL_FIERY_BOULDER_4

    и сразу начинаем двигаеться от позиции
    Position:       2297.642 Y: -204.163, 212.631    Flags: Flying (512)

    по позициям
    [0] Points:     2208.351 Y: -208.3108,213.9586

    [0] WayPoints:  2296.247 Y: -204.237, 213.545
    [1] WayPoints:  2249.747 Y: -205.987, 213.545

    далее кастуем на себя спелл SPELL_ALPHA_FADE_OUT
    */
};

Position const leftBoulder[] =    //< NPC_FIERY_BOULDER_LEFT
{
    {2298.648f, -218.293f, 253.099f},   // стартовая позиция Falling (64) 
    {2298.648f, -218.293f, 212.631f},   // позиция 2 - упали в низ, как только достигаем позиции вешаем Flags: Flying (512) 
    {2294.631f, -218.836f, 213.221f},   // [0] WayPoints точка старта от босса
    {2251.131f, -220.086f, 213.471f}    // [1] WayPoints точка на другой стороне моста

    /*

    Кастуем сами на себя спелл SPELL_FIERY_BOULDER_1 и получаем триггером SPELL_FIERY_BOULDER_2

    Стартовая позиция:
    Position:       2298.648 Y: -218.293, 253.099      Flags: Falling (64)

    прямо от неё мы двигаемся сюда
    Points:         2298.648 Y: -218.293, 212.631

    через 2 секунды кастуем на себя SPELL_FIERY_BOULDER_3 и получаем триггером SPELL_FIERY_BOULDER_5

    и сразу начинаем двигаеться от позиции
    Position:       2298.648 Y: -218.293, 212.631     Flags: Flying (512)
    по позициям
    [0] Points:     2209.615 Y: -221.880, 214.312

    [0] WayPoints:  2294.631 Y: -218.836, 213.2216
    [1] WayPoints:  2251.131 Y: -220.086, 213.4716

    далее кастуем на себя спелл SPELL_ALPHA_FADE_OUT

    */
};

Position const middleBoulder[] =  //< NPC_FIERY_BOULDER_MIDDLE
{
    {2297.642f, -211.868f, 253.099f},   // стартовая позиция Falling (64) 
    {2297.642f, -211.868f, 212.667f},   // позиция 2 - упали в низ, как только достигаем позиции вешаем Flags: Flying (512), после достижения позиции идёт каст SPELL_FIERY_BOULDER_3
    {2295.813f, -212.111f, 213.075f},   // [0] WayPoints точка старта от босса
    {2247.813f, -212.111f, 213.575f},   // [1] WayPoints точка на другой стороне моста
    {2212.783f, -212.335f, 213.937f}    // точка перед отправкой назад

    /*

    Кастуем сами на себя спелл SPELL_FIERY_BOULDER_1 и получаем триггером SPELL_FIERY_BOULDER_2

    Стартовая позиция:
    Position:       2297.642, -211.868, 253.099     Flags: Falling (64)

    прямо от неё мы двигаемся сюда
    [0] Points:     2297.642, -211.8681, 212.667

    через 2 секунды кастуем на себя SPELL_FIERY_BOULDER_3 и получаем триггером SPELL_FIERY_BOULDER_6

    и сразу начинаем двигаеться от позиции
    Position:       2297.642, -211.868, 212.667    Flags: Flying (512)

    по позициям
    [0] Points:     2209.984, -212.354, 213.983

    [0] WayPoints:  2295.813, -212.111, 213.075
    [1] WayPoints:  2247.813, -212.111, 213.575

    далее кастуем на себя спелл SPELL_FIERY_BOULDER_7

    далее в позиции
    Position:       2212.783, -212.3358, 213.937    Flags: None (0)

    и после этого кастуем на себя  SPELL_ALPHA_FADE_OUT

    */
};

class boss_roltall : public CreatureScript
{
public:
    boss_roltall() : CreatureScript("boss_roltall") { }

    struct boss_roltallAI : public BossAI
    {
        boss_roltallAI(Creature* creature) : BossAI(creature, DATA_ROLTALL), summons(me)
        {
            instance = me->GetInstanceScript();

            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            SetCombatMovement(false);
        }

        void Reset()
        {
            _Reset();
            summons.DespawnAll();

            bouldersCounter = 0;
            burningSlagCounter = 0;
        }

        void EnterCombat(Unit* /*who*/)
        {
            _EnterCombat();

            events.RescheduleEvent(EVENT_1, 1 * IN_MILLISECONDS);
        }

        void JustDied(Unit* /*killer*/)
        {
            _JustDied();
            summons.DespawnAll();
        }

        void EnterEvadeMode()
        {
            CreatureAI::EnterEvadeMode();
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
            summon->AI()->DoAction(ACTION_1);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (me->GetDistance(me->getVictim()) >= 120.0f)
            {
                EnterEvadeMode();
                return;
            }

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        if (bouldersCounter < 3)
                        {
                            events.RescheduleEvent(EVENT_1, 0.5 * IN_MILLISECONDS);
                            events.RescheduleEvent(EVENT_7 + urand(0, 2), 0.5 * IN_MILLISECONDS);
                            DoCast(SPELL_FIERY_BOULDER);
                            ++bouldersCounter;
                        } else
                        {
                            bouldersCounter = 0;
                            events.RescheduleEvent(EVENT_4, 0.5 * IN_MILLISECONDS);
                        }
                        break;
                    case EVENT_7: // {0, NPC_FIERY_BOULDER_RIGHT, DATA_BOULDER_R}
                        if (instance->GetData(DATA_BOULDER_R) != DONE)
                        {
                            instance->SetData(DATA_BOULDER_R, DONE);
                            me->SummonCreature(NPC_FIERY_BOULDER_RIGHT, rightBoulder[0]);
                        } else if (instance->GetData(DATA_BOULDER_L) != DONE || instance->GetData(DATA_BOULDER_M) != DONE)
                            if (urand(0, 1))
                                events.RescheduleEvent(EVENT_8, 0.5 * IN_MILLISECONDS);
                            else
                                events.RescheduleEvent(EVENT_9, 0.5 * IN_MILLISECONDS);
                        break;
                    case EVENT_8: // {1, NPC_FIERY_BOULDER_LEFT, DATA_BOULDER_L}
                        if (instance->GetData(DATA_BOULDER_L) != DONE)
                        {
                            instance->SetData(DATA_BOULDER_L, DONE);
                            me->SummonCreature(NPC_FIERY_BOULDER_LEFT, leftBoulder[0]);
                        } else if (instance->GetData(DATA_BOULDER_R) != DONE || instance->GetData(DATA_BOULDER_M) != DONE)
                            if (urand(0, 1))
                                events.RescheduleEvent(EVENT_7, 0.5 * IN_MILLISECONDS);
                            else
                                events.RescheduleEvent(EVENT_9, 0.5 * IN_MILLISECONDS);
                        break;
                    case EVENT_9: // {2, NPC_FIERY_BOULDER_MIDDLE, DATA_BOULDER_M}
                        if (instance->GetData(DATA_BOULDER_M) != DONE)
                        {
                            instance->SetData(DATA_BOULDER_M, DONE);
                            me->SummonCreature(NPC_FIERY_BOULDER_MIDDLE, middleBoulder[0]);
                        } else if (instance->GetData(DATA_BOULDER_R) != DONE || instance->GetData(DATA_BOULDER_L) != DONE)
                            if (urand(0, 1))
                                events.RescheduleEvent(EVENT_7, 0.5 * IN_MILLISECONDS);
                            else
                                events.RescheduleEvent(EVENT_8, 0.5 * IN_MILLISECONDS);
                        break;
                    case EVENT_4:
                        if (burningSlagCounter < 6)
                        {
                            events.RescheduleEvent(EVENT_4, 2 * IN_MILLISECONDS);
                            DoCast(SPELL_BURNING_SLAG); // should trigger SPELL_BURNING_SLAG_2
                            ++burningSlagCounter;
                        } else
                        {
                            events.RescheduleEvent(EVENT_5, 10 * IN_MILLISECONDS);
                            burningSlagCounter = 0;
                        }
                        break;
                    case EVENT_5:
                        events.RescheduleEvent(EVENT_1, 10 * IN_MILLISECONDS);
                        me->CastSpell(me, SPELL_HEAT_WAVE_2);
                        if (Creature* heatWave = me->FindNearestCreature(NPC_HEAT_WAVE, 30.0f))
                            heatWave->AI()->DoAction(ACTION_1);
                        break;
                    default:
                        break;
                }
            }

            if (!me->IsWithinMeleeRange(me->getVictim()))
                DoSpellAttackIfReady(SPELL_BURNING_SLAG);
            else
                DoMeleeAttackIfReady();
        }

    private:
        InstanceScript* instance;
        EventMap events;
        SummonList summons;
        uint8 bouldersCounter;
        uint8 burningSlagCounter;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_roltallAI(creature);
    }
};

class npc_fiery_boulder_right : public CreatureScript
{
public:
    npc_fiery_boulder_right() : CreatureScript("npc_fiery_boulder_right") { }

    struct npc_fiery_boulder_rightAI : public ScriptedAI
    {
        npc_fiery_boulder_rightAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetFloatValue(UNIT_FIELD_BOUNDING_RADIUS, 0.5f);
            me->SetFloatValue(UNIT_FIELD_COMBAT_REACH, 1.0f);
            me->SetDisplayId(11686);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_ATTACKABLE_1);
        }

        void DoAction(int32 const action) override
        {
            switch (action)
            {
                case ACTION_1:
                    events.RescheduleEvent(EVENT_1, 0.1 * IN_MILLISECONDS);
                    break;
                default:
                    break;
            }
        }

        void Reset() override
        {
            events.Reset();

            checkTimer = 0;
        }

        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);

            if (checkTimer <= diff)
            {
                if (me->GetExactDist2d(rightBoulder[1].GetPositionX(), rightBoulder[1].GetPositionY()) < 3.0f)
                {
                    me->AddAura(SPELL_ALPHA_FADE_OUT, me);
                    me->DespawnOrUnsummon(1 * IN_MILLISECONDS);
                }

                checkTimer = 2 * IN_MILLISECONDS;
            } else
                checkTimer -= diff;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                    {
                        events.RescheduleEvent(EVENT_2, 1.5 * IN_MILLISECONDS);
                        me->CastSpell(me->GetPosition(), SPELL_FIERY_BOULDER_4, false);

                        Movement::MoveSplineInit init(*me);
                        init.SetFall();
                        init.MoveTo(rightBoulder[1]);
                        init.Launch();
                        break;
                    }
                    case EVENT_2:
                    {
                        events.RescheduleEvent(EVENT_3, 2 * IN_MILLISECONDS);

                        Movement::MoveSplineInit init(*me);
                        init.SetFly();
                        init.SetSmooth();
                        init.MoveTo(rightBoulder[2]);
                        init.Launch();
                        break;
                    }
                    case EVENT_3:
                    {
                        events.RescheduleEvent(EVENT_4, 2 * IN_MILLISECONDS);
                        Movement::MoveSplineInit init(*me);
                        init.SetFly();
                        init.SetSmooth();
                        init.MoveTo(rightBoulder[3]);
                        init.Launch();

                        //me->RemoveAura(SPELL_FIERY_BOULDER_1);
                        //me->AddAura(SPELL_FIERY_BOULDER_1, me);
                        //me->AddAura(SPELL_FIERY_BOULDER_3, me);
                        //me->AddAura(SPELL_FIERY_BOULDER_4, me);
                        break;
                    }
                    case EVENT_4:
                    {
                        Movement::MoveSplineInit init(*me);
                        for (uint8 i = 3; i > 0; --i)
                            init.Path().push_back(G3D::Vector3(rightBoulder[i].GetPositionX(), rightBoulder[i].GetPositionY(), rightBoulder[i].GetPositionZ()));
                        init.SetSmooth();
                        init.Launch();
                        break;
                    }
                    default:
                        break;
                }
            }
        }

    private:
        EventMap events;
        uint32 checkTimer;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_fiery_boulder_rightAI(creature);
    }
};

class npc_fiery_boulder_left : public CreatureScript
{
public:
    npc_fiery_boulder_left() : CreatureScript("npc_fiery_boulder_left") { }

    struct npc_fiery_boulder_leftAI : public ScriptedAI
    {
        npc_fiery_boulder_leftAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetFloatValue(UNIT_FIELD_BOUNDING_RADIUS, 0.5f);
            me->SetFloatValue(UNIT_FIELD_COMBAT_REACH, 1.0f);
            me->SetDisplayId(11686);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_ATTACKABLE_1);
        }

        void DoAction(int32 const action) override
        {
            switch (action)
            {
                case ACTION_1:
                    events.RescheduleEvent(EVENT_1, 0.1 * IN_MILLISECONDS);
                    break;
                default:
                    break;
            }
        }

        void Reset() override
        {
            events.Reset();

            checkTimer = 0;
        }

        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);

            if (checkTimer <= diff)
            {
                if (me->GetExactDist2d(leftBoulder[1].GetPositionX(), leftBoulder[1].GetPositionY()) < 3.0f)
                {
                    me->AddAura(SPELL_ALPHA_FADE_OUT, me);
                    me->DespawnOrUnsummon(1 * IN_MILLISECONDS);
                }

                checkTimer = 0.5 * IN_MILLISECONDS;
            } else
                checkTimer -= diff;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                    {
                        events.RescheduleEvent(EVENT_2, 1.5 * IN_MILLISECONDS);
                        me->CastSpell(me->GetPosition(), SPELL_FIERY_BOULDER_4, false);

                        Movement::MoveSplineInit init(*me);
                        init.SetFall();
                        init.MoveTo(leftBoulder[1]);
                        init.Launch();
                        break;
                    }
                    case EVENT_2:
                    {
                        events.RescheduleEvent(EVENT_3, 2 * IN_MILLISECONDS);

                        Movement::MoveSplineInit init(*me);
                        init.SetFly();
                        init.SetSmooth();
                        init.MoveTo(leftBoulder[2]);
                        init.Launch();
                        break;
                    }
                    case EVENT_3:
                    {
                        events.RescheduleEvent(EVENT_4, 2 * IN_MILLISECONDS);
                        Movement::MoveSplineInit init(*me);
                        init.SetFly();
                        init.SetSmooth();
                        init.MoveTo(leftBoulder[3]);
                        init.Launch();

                        //me->RemoveAura(SPELL_FIERY_BOULDER_1);
                        //me->AddAura(SPELL_FIERY_BOULDER_1, me);
                        //me->AddAura(SPELL_FIERY_BOULDER_3, me);
                        //me->AddAura(SPELL_FIERY_BOULDER_4, me);
                        break;
                    }
                    case EVENT_4:
                    {
                        Movement::MoveSplineInit init(*me);
                        for (uint8 i = 4; i > 0; --i)
                            init.Path().push_back(G3D::Vector3(leftBoulder[i].GetPositionX(), leftBoulder[i].GetPositionY(), leftBoulder[i].GetPositionZ()));
                        init.SetSmooth();
                        init.Launch();
                        break;
                    }
                    default:
                        break;
                }
            }
        }

    private:
        EventMap events;
        uint32 checkTimer;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_fiery_boulder_leftAI(creature);
    }
};

class npc_fiery_boulder_middle : public CreatureScript
{
public:
    npc_fiery_boulder_middle() : CreatureScript("npc_fiery_boulder_middle") { }

    struct npc_fiery_boulder_middleAI : public ScriptedAI
    {
        npc_fiery_boulder_middleAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetFloatValue(UNIT_FIELD_BOUNDING_RADIUS, 0.5f);
            me->SetFloatValue(UNIT_FIELD_COMBAT_REACH, 1.0f);
            me->SetDisplayId(11686);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_ATTACKABLE_1);
        }

        void DoAction(int32 const action) override
        {
            switch (action)
            {
                case ACTION_1:
                    events.RescheduleEvent(EVENT_1, 0.1 * IN_MILLISECONDS);
                    break;
                default:
                    break;
            }
        }

        void Reset() override
        {
            events.Reset();

            checkTimer = 0;
        }

        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);

            if (checkTimer <= diff)
            {
                if (me->GetExactDist2d(middleBoulder[1].GetPositionX(), middleBoulder[1].GetPositionY()) < 3.0f)
                {
                    me->AddAura(SPELL_ALPHA_FADE_OUT, me);
                    me->DespawnOrUnsummon(1 * IN_MILLISECONDS);
                }

                checkTimer = 0.5 * IN_MILLISECONDS;
            } else
                checkTimer -= diff;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                    {
                        events.RescheduleEvent(EVENT_2, 1.5 * IN_MILLISECONDS);
                        me->CastSpell(me->GetPosition(), SPELL_FIERY_BOULDER_4, false);

                        Movement::MoveSplineInit init(*me);
                        init.SetFall();
                        init.MoveTo(middleBoulder[1]);
                        init.Launch();
                        break;
                    }
                    case EVENT_2:
                    {
                        events.RescheduleEvent(EVENT_3, 2 * IN_MILLISECONDS);

                        Movement::MoveSplineInit init(*me);
                        init.SetFly();
                        init.SetSmooth();
                        init.MoveTo(middleBoulder[2]);
                        init.Launch();
                        break;
                    }
                    case EVENT_3:
                    {
                        events.RescheduleEvent(EVENT_4, 2 * IN_MILLISECONDS);
                        Movement::MoveSplineInit init(*me);
                        init.SetFly();
                        init.SetSmooth();
                        init.MoveTo(middleBoulder[3]);
                        init.Launch();
                        break;
                    }
                    case EVENT_4:
                    {
                        events.RescheduleEvent(EVENT_5, 2 * IN_MILLISECONDS);
                        Movement::MoveSplineInit init(*me);
                        init.SetFly();
                        init.SetSmooth();
                        init.MoveTo(middleBoulder[3]);
                        init.Launch();

                        //me->RemoveAura(SPELL_FIERY_BOULDER_1);
                        //me->AddAura(SPELL_FIERY_BOULDER_1, me);
                        //me->AddAura(SPELL_FIERY_BOULDER_3, me);
                        //me->AddAura(SPELL_FIERY_BOULDER_4, me);
                        break;
                    }
                    case EVENT_5:
                    {
                        Movement::MoveSplineInit init(*me);
                        for (uint8 i = 4; i > 0; --i)
                            init.Path().push_back(G3D::Vector3(middleBoulder[i].GetPositionX(), middleBoulder[i].GetPositionY(), middleBoulder[i].GetPositionZ()));
                        init.SetSmooth();
                        init.Launch();
                        break;
                    }
                    default:
                        break;
                }
            }
        }

    private:
        EventMap events;
        uint32 checkTimer;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_fiery_boulder_middleAI(creature);
    }
};

class npc_heat_wave : public CreatureScript
{
public:
    npc_heat_wave() : CreatureScript("npc_heat_wave") { }

    struct npc_heat_waveAI : public ScriptedAI
    {
        npc_heat_waveAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_ATTACKABLE_1);
        }

        void DoAction(int32 const action) override
        {
            switch (action)
            {
                case ACTION_1:
                    events.RescheduleEvent(EVENT_1, 0.1 * IN_MILLISECONDS);
                    break;
                default:
                    break;
            }
        }

        void Reset() override
        {
            events.Reset();
        }

        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        me->CastSpell(me, SPELL_HEAT_WAVE_3);
                        me->CastSpell(2312.793f, -211.3021f, 217.048f, SPELL_HEAT_WAVE_4);
                        me->CastSpell(me, SPELL_HEAT_WAVE);
                        break;
                    default:
                        break;
                }
            }
        }

    private:
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_heat_waveAI(creature);
    }
};

void AddSC_boss_roltall()
{
    new boss_roltall();

    new npc_fiery_boulder_right();
    new npc_fiery_boulder_left();
    new npc_fiery_boulder_middle();
    new npc_heat_wave();
}
