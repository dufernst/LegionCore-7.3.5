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

enum eSpells
{
    SPELL_SONG_OF_THE_IMPERATOR     = 123793,
    SPELL_PHEROMONES_OF_ZEAL        = 123812,
    SPELL_PHEROMONES_OF_ZEAL_BUFF   = 123833,
    SPELL_GAS_VISUAL                = 123811,
    SPELL_SONG_OF_THE_EMPRESS       = 123791,
    SPELL_SONG_OF_THE_EMPRESS_RANGE = 130133,
    SPELL_INHALE                    = 122852,
    SPELL_EXHALE                    = 122761,
    //Platform 1
    SPELL_NOISE_CANCELLING          = 122707,
    SPELL_FORCE_AND_VERVE           = 122713,
    //Platform 2
    SPELL_ATTENUATION               = 122496,
    SPELL_BERSERK                   = 47008,

    //Other
    SPELL_SONIC_RING_VISUAL         = 122334,
    SPELL_SONIC_RING_VISUAL_H       = 124668,
};

enum eEvents
{
    //Vizier Zorlok
    EVENT_MELEE_CHECK         = 1,
    EVENT_GO_LAST_POS         = 2,
    EVENT_GO_NEXT_PLATFORM    = 3,
    EVENT_INHALE              = 4,
    EVENT_EXHALE              = 5,
    //Platform 1
    EVENT_FORCE_AND_VERVE     = 6,
    //Platform 2
    EVENT_ATTENUATION         = 7,

    //Gas Controller
    EVENT_CHECK_PLAYERS       = 1,
};

enum Actions
{
    //Gas Controller
    ACTION_GAS_ON             = 1,
    ACTION_GAS_OFF            = 2,
};

Position const centerpos = {-2275.27f, 259.1f, 415.34f}; //In air
Position const centerlandpos = {-2275.41f, 258.33f, 406.38f}; //In land

Position const platformpos[3] = 
{
    {-2312.74f, 298.69f, 409.89f}, 
    {-2235.27f, 217.29f, 409.89f}, 
    {-2312.05f, 221.74f, 409.89f}, 
};

float const curplpos = 409.149f;

class boss_vizier_zorlok : public CreatureScript
{
    public:
        boss_vizier_zorlok() : CreatureScript("boss_vizier_zorlok") {}

        struct boss_vizier_zorlokAI : public BossAI
        {
            boss_vizier_zorlokAI(Creature* creature) : BossAI(creature, DATA_VIZIER_ZORLOK)
            {
                me->SetCanFly(true);
                me->SetDisableGravity(true);
                me->SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
            }

            uint8 newindex, lastindex, flycount;
            uint8 InhaleCount;
            uint32 berserkTimer;
            bool flyMove;

            void Reset()
            {
                _Reset();
                events.Reset();
                GasControl(false);
                FlyControl(true);
                flyMove = false;
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                me->RemoveAurasDueToSpell(SPELL_PHEROMONES_OF_ZEAL_BUFF);
                me->RemoveAurasDueToSpell(SPELL_INHALE);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_PHEROMONES_OF_ZEAL);
                instance->DoRemoveAurasDueToSpellOnPlayers(122706); // AT buff
                newindex = 0;
                lastindex = urand(0, 2);
                flycount = 0;
                InhaleCount = 0;
                berserkTimer = 60 * MINUTE * IN_MILLISECONDS;
            }

            void GasControl(bool state)
            {
                if (instance)
                {
                    if (state)
                    {
                        //if (Creature* gc = me->GetCreature(*me, instance->GetData64(NPC_GAS_CONTROLLER)))
                        //    gc->AI()->DoAction(ACTION_GAS_ON);
                        DoCast(me, SPELL_GAS_VISUAL, true);
                    }
                    else
                    {
                        //if (Creature* gc = me->GetCreature(*me, instance->GetData64(NPC_GAS_CONTROLLER)))
                        //    gc->AI()->DoAction(ACTION_GAS_OFF);
                        me->RemoveAllAreaObjects();
                    }
                }
            }
            
            void FlyControl(bool apply)
            {
                if (apply)
                {
                    flyMove = true;
                    me->AttackStop();
                    me->SetReactState(REACT_PASSIVE);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                    me->SetCanFly(true);
                    me->SetDisableGravity(true);
                    me->SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                }
                else
                {
                    flyMove = false;
                    me->SetCanFly(false);
                    me->SetDisableGravity(false);
                    me->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                }
            }
            
            void EnterCombat(Unit* /*who*/)
            {
                if (instance)
                    instance->SetBossState(DATA_VIZIER_ZORLOK, IN_PROGRESS);

                FlyControl(true);
                GasControl(true);
                GoNextRandomPlatform();
            }
            
            void MovementInform(uint32 type, uint32 id)
            {
                if (type == POINT_MOTION_TYPE)
                {
                    if (id == 1)
                    {
                        FlyControl(false);
                        if (flycount < 3)
                            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                        else
                        {
                            events.ScheduleEvent(EVENT_MELEE_CHECK, 2000);
                            events.ScheduleEvent(EVENT_INHALE, 17000);
                            events.ScheduleEvent(EVENT_EXHALE, 17000);
                            events.ScheduleEvent(EVENT_FORCE_AND_VERVE, 16000);
                            events.ScheduleEvent(EVENT_ATTENUATION, 22000);
                            GasControl(false);
                            DoCast(me, SPELL_PHEROMONES_OF_ZEAL_BUFF, true);
                        }
                        me->RemoveAurasDueToSpell(SPELL_INHALE);
                        me->SetReactState(REACT_AGGRESSIVE);
                        DoCast(SPELL_SONG_OF_THE_EMPRESS);
                        DoZoneInCombat(me, 150.0f);
                    }
                }
            }
            
            void GoNextRandomPlatform()
            {
                events.Reset();
                events.ScheduleEvent(EVENT_MELEE_CHECK, 500);
                events.ScheduleEvent(EVENT_INHALE, 17000);
                events.ScheduleEvent(EVENT_EXHALE, 17000);

                do
                {
                    newindex = urand(0, 2);
                }
                while (newindex == lastindex);

                switch (newindex)
                {
                    case 0:
                        events.ScheduleEvent(EVENT_FORCE_AND_VERVE, 16000);
                        break;
                    case 1:
                        events.ScheduleEvent(EVENT_ATTENUATION, 22000);
                        break;
                    case 2:
                        //MK
                        break;
                }

                lastindex = newindex;
                // In future send newindex in id point, for specific platform events.
                me->GetMotionMaster()->MovePoint(1, platformpos[newindex].GetPositionX(), platformpos[newindex].GetPositionY(), platformpos[newindex].GetPositionZ());
            }

            void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
            {
                if (me->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
                    if (me->GetCurrentSpell(CURRENT_CHANNELED_SPELL)->m_spellInfo->Id == SPELL_SONG_OF_THE_EMPRESS
                        || me->GetCurrentSpell(CURRENT_CHANNELED_SPELL)->m_spellInfo->Id == SPELL_SONG_OF_THE_EMPRESS_RANGE)
                        if (me->IsWithinMeleeRange(me->getVictim()))
                            me->InterruptSpell(CURRENT_CHANNELED_SPELL);

                if (HealthBelowPct(80) && !flycount ||
                    HealthBelowPct(60) && flycount == 1)
                {
                    flycount++;
                    FlyControl(true);
                    me->GetMotionMaster()->MoveJump(centerpos.GetPositionX(), centerpos.GetPositionY(), centerpos.GetPositionZ(), 10.0f, 10.0f);
                    events.ScheduleEvent(EVENT_GO_NEXT_PLATFORM, 3000);
                }
                else if (HealthBelowPct(40) && flycount == 2)
                {
                    flycount++;
                    FlyControl(true);
                    GasControl(false);
                    me->GetMotionMaster()->MoveJump(centerpos.GetPositionX(), centerpos.GetPositionY(), centerpos.GetPositionZ(), 10.0f, 10.0f);
                    events.ScheduleEvent(EVENT_GO_LAST_POS, 3000);
                }
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();

                Map::PlayerList const& players = me->GetMap()->GetPlayers();
                if (!players.isEmpty())
                {
                    for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                    {
                        if (Player* pPlayer = itr->getSource())
                            me->GetMap()->ToInstanceMap()->PermBindAllPlayers(pPlayer);
                    }
                }
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (berserkTimer <= diff)
                {
                    DoCast(me, SPELL_BERSERK, true);
                    berserkTimer = 10 * MINUTE * IN_MILLISECONDS;
                }
                else berserkTimer -= diff;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_MELEE_CHECK:
                            if (me->getVictim())
                                if (!me->IsWithinMeleeRange(me->getVictim()) && !flyMove)
                                    DoCast(me, SPELL_SONG_OF_THE_EMPRESS_RANGE, true);
                            events.ScheduleEvent(EVENT_MELEE_CHECK, 500);
                            break;
                        case EVENT_GO_NEXT_PLATFORM:
                            GoNextRandomPlatform();
                            break;
                        case EVENT_GO_LAST_POS:
                            me->GetMotionMaster()->MovePoint(1, centerlandpos.GetPositionX(), centerlandpos.GetPositionY(), centerlandpos.GetPositionZ());
                            events.Reset();
                            break;
                        case EVENT_INHALE:
                            DoCast(SPELL_INHALE);
                            InhaleCount++;
                            events.ScheduleEvent(EVENT_INHALE, 18000);
                            break;
                        case EVENT_EXHALE:
                            if (InhaleCount > 1)
                            {
                                InhaleCount = 0;
                                DoCast(SPELL_EXHALE);
                            }
                            events.ScheduleEvent(EVENT_EXHALE, 18000);
                            break;
                        case EVENT_FORCE_AND_VERVE:
                            for (uint8 i = 0; i < 3; i++)
                            {
                                float angle = urand(0, 6);
                                float distance = 7 * (i + 0.5);
                                float x = me->GetPositionX() + distance * std::cos(angle);
                                float y = me->GetPositionY() + distance * std::sin(angle);
                                me->CastSpell(x, y, me->GetPositionZ(), SPELL_NOISE_CANCELLING, true);
                            }
                            DoCast(SPELL_FORCE_AND_VERVE);
                            events.ScheduleEvent(EVENT_FORCE_AND_VERVE, 34000);
                            break;
                        case EVENT_ATTENUATION:
                            DoCast(SPELL_ATTENUATION);
                            events.ScheduleEvent(EVENT_ATTENUATION, 40000);
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_vizier_zorlokAI(creature);
        }
};

class npc_gas_controller : public CreatureScript
{
    public:
        npc_gas_controller() : CreatureScript("npc_gas_controller") {}

        struct npc_gas_controllerAI : public ScriptedAI
        {
            npc_gas_controllerAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
                me->SetReactState(REACT_PASSIVE);
                me->SetDisplayId(11686);
                gaseoff = false;
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            }

            InstanceScript* instance;
            EventMap events;
            bool gaseoff;

            void Reset(){}
            
            void DoAction(const int32 action)
            {
                switch (action)
                {
                    case ACTION_GAS_ON:
                        DoCast(me, SPELL_GAS_VISUAL);
                        events.ScheduleEvent(EVENT_CHECK_PLAYERS, 1000);
                        break;
                    case ACTION_GAS_OFF:
                        gaseoff = true;
                        me->RemoveAllAreaObjects();
                        break;
                }
            }

            void CheckPlayers()
            {
                if (Map* map = me->GetMap())
                {
                    if (map->IsDungeon())
                    {
                        Map::PlayerList const &players = map->GetPlayers();
                        for (Map::PlayerList::const_iterator i = players.begin(); i != players.end(); ++i)
                        {
                            if (Player* pl = i->getSource())
                            {
                                if (pl->isAlive() && pl->GetPositionZ() < curplpos)
                                {
                                    if (!pl->HasAura(SPELL_PHEROMONES_OF_ZEAL))
                                        me->CastSpell(pl, SPELL_PHEROMONES_OF_ZEAL, true);
                                }
                                else if (pl->isAlive() && pl->GetPositionZ() >= curplpos)
                                    pl->RemoveAurasDueToSpell(SPELL_PHEROMONES_OF_ZEAL);
                            }
                        }

                        if (gaseoff)
                        {
                            gaseoff = false;
                            events.CancelEvent(EVENT_CHECK_PLAYERS);
                            if (instance)
                                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_PHEROMONES_OF_ZEAL);
                        }
                        else
                            events.ScheduleEvent(EVENT_CHECK_PLAYERS, 1000);
                    }
                }
            }
            
            void EnterEvadeMode(){}

            void EnterCombat(Unit* who){}

            void UpdateAI(uint32 diff)
            {
                events.Update(diff);

                switch (events.ExecuteEvent())
                {
                case EVENT_CHECK_PLAYERS:
                    CheckPlayers();
                    break;
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_gas_controllerAI(creature);
        }
};

//62689, 62716, 62717, 62743, 62744
class npc_zorlok_sonic_ring : public CreatureScript
{
    public:
        npc_zorlok_sonic_ring() : CreatureScript("npc_zorlok_sonic_ring") {}

        struct npc_zorlok_sonic_ringAI : public ScriptedAI
        {
            npc_zorlok_sonic_ringAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            }

            InstanceScript* instance;

            void Reset(){}
            
            void IsSummonedBy(Unit* summoner)
            {
                DoCast(me, SPELL_SONIC_RING_VISUAL, true);
                if (me->GetMap()->IsHeroic())
                    DoCast(me, SPELL_SONIC_RING_VISUAL_H, true);
                Position pos;
                me->GetRandomNearPosition(pos, 40);
                me->GetMotionMaster()->MovePoint(1, pos);
            }
            
            void MovementInform(uint32 type, uint32 id)
            {
                if (type != POINT_MOTION_TYPE)
                    return;

                if (id == 1)
                    me->DespawnOrUnsummon();
            }

            void UpdateAI(uint32 diff) {}
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_zorlok_sonic_ringAI(creature);
        }
};

void AddSC_boss_vizier_zorlok()
{
    new boss_vizier_zorlok();
    new npc_gas_controller();
    new npc_zorlok_sonic_ring();
}
