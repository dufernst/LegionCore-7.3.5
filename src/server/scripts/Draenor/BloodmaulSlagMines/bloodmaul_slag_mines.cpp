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

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "bloodmaul_slag_mines.h"

enum Spells
{
    //< http://www.wowhead.com/npc=81750/bloodmaul-ogron
    SPELL_COLOSAL_ROAR              = 164582,
    SPELL_MASSIVE_STOMP             = 164587,

    //< http://www.wowhead.com/npc=81767/bloodmaul-flamespeaker
    SPELL_EXPLODING_FLAMES          = 164617,
    SPELL_CHANEL_FLAMES             = 164615, //< Area Trigger (2593)
    SPELL_CHANEL_FLAMES_2           = 164616,

    //< http://www.wowhead.com/npc=75194/gnasher
    SPELL_FURIOUS_SWIPES            = 164624,
    SPELL_RENDING_CHARGE            = 164641,

    //< http://www.wowhead.com/npc=75211/magma-lord
    SPELL_FIREBALL                  = 152427,
    SPELL_PILAR_OF_FLAMES           = 151623,

    //< http://www.wowhead.com/npc=74349/bloodmaul-magma-binder
    SPELL_LAVA_BURST                = 151558,
    SPELL_MOLTEN_BINDING            = 151566,

    //< http://www.wowhead.com/npc=75406/slagna
    SPELL_LAVA_SPIT                 = 152183,

    //< http://www.wowhead.com/npc=75209/molten-earth-elemental
    SPELL_LAVA_ARC                  = 151720,

    //< http://www.wowhead.com/npc=75210/bloodmaul-warder
    SPELL_CLEAVE                    = 40505,
    SPELL_FRIGHTENING_ROAR          = 151545,

    //< http://www.wowhead.com/npc=75272/bloodmaul-ogre-mage
    SPELL_BLOOD_RAGE                = 151548,

    //< http://www.wowhead.com/npc=75426/bloodmaul-overseer
    SPELL_SUBJUGATE                 = 151697,
    SPELL_SUPPRESSION_FIELD         = 151581,

    //< http://www.wowhead.com/npc=84978/bloodmaul-enforcer
    SPELL_BEATDOWN                  = 151415,
    SPELL_CRUSH                     = 151447,
    SPELL_LUMBERING_LEAP            = 151542,

    //< http://www.wowhead.com/npc=75191/bloodmaul-slaver
    SPELL_CHAIN_GRIP                = 151990,
    SPELL_SHOCK_BOLA                = 152073,
    SPELL_SLAVERS_RAGE              = 151965,
    SPELL_VICIOUS_SLASH             = 152043,

    //< http://www.wowhead.com/npc=75820/vengeful-magma-elemental
    SPELL_ARMOR_DENT                = 151685,
    SPELL_CINDER_SPLASH             = 152298,
    SPELL_FIREBALL_2                = 77508,
    
    //< http://www.wowhead.com/npc=75198/bloodmaul-geomancer
    SPELL_STONE_BOLT                = 164592,
    SPELL_STONE_BULWARK             = 164597,
};

class npc_bloodmaul_ogron : public CreatureScript
{
public:
    npc_bloodmaul_ogron() : CreatureScript("npc_bloodmaul_ogron") { }

    struct npc_bloodmaul_ogronAI : public ScriptedAI
    {
        npc_bloodmaul_ogronAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap events;

        void Reset()
        {
            events.Reset();
            events.RescheduleEvent(EVENT_1, 9 * IN_MILLISECONDS);
            events.RescheduleEvent(EVENT_2, urand(2, 7) * IN_MILLISECONDS);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        events.RescheduleEvent(EVENT_1, urand(10, 15) * IN_MILLISECONDS);
                        me->CastSpell(me, SPELL_MASSIVE_STOMP);
                        break;
                    case EVENT_2:
                        events.RescheduleEvent(EVENT_2, urand(4, 12) * IN_MILLISECONDS);
                        me->CastSpell(me, SPELL_COLOSAL_ROAR);
                        break;
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bloodmaul_ogronAI(creature);
    }
};

class npc_bloodmaul_flamespeaker : public CreatureScript
{
public:
    npc_bloodmaul_flamespeaker() : CreatureScript("npc_bloodmaul_flamespeaker") { }

    struct npc_bloodmaul_flamespeakerAI : public ScriptedAI
    {
        npc_bloodmaul_flamespeakerAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap events;

        void Reset()
        {
            events.Reset();
            events.RescheduleEvent(EVENT_1, urand(2, 5) * IN_MILLISECONDS);
            events.RescheduleEvent(EVENT_2, urand(7, 13) * IN_MILLISECONDS);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        events.RescheduleEvent(EVENT_1, urand(5, 14) * IN_MILLISECONDS);
                        me->CastSpell(me, SPELL_EXPLODING_FLAMES);
                        break;
                    case EVENT_2:
                        events.RescheduleEvent(EVENT_2, urand(7, 13) * IN_MILLISECONDS);
                        me->CastSpell(me, SPELL_CHANEL_FLAMES);
                        me->CastSpell(me, SPELL_CHANEL_FLAMES_2);
                        break;
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bloodmaul_flamespeakerAI(creature);
    }
};

class npc_bloodmaul_gnasher : public CreatureScript
{
public:
    npc_bloodmaul_gnasher() : CreatureScript("npc_bloodmaul_gnasher") { }

    struct npc_bloodmaul_gnasherAI : public ScriptedAI
    {
        npc_bloodmaul_gnasherAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap events;

        void Reset()
        {
            events.Reset();
            events.RescheduleEvent(EVENT_1, urand(1, 3) * IN_MILLISECONDS);
            events.RescheduleEvent(EVENT_2, urand(5, 10) * IN_MILLISECONDS);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        events.RescheduleEvent(EVENT_1, urand(5, 14) * IN_MILLISECONDS);
                        me->CastSpell(SelectTarget(SELECT_TARGET_RANDOM), SPELL_RENDING_CHARGE);
                        break;
                    case EVENT_2:
                        events.RescheduleEvent(EVENT_2, urand(7, 13) * IN_MILLISECONDS);
                        DoCastVictim(SPELL_FURIOUS_SWIPES);
                        break;
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bloodmaul_gnasherAI(creature);
    }
};

class npc_bloodmaul_magma_lord : public CreatureScript
{
public:
    npc_bloodmaul_magma_lord() : CreatureScript("npc_bloodmaul_magma_lord") { }

    struct npc_bloodmaul_magma_lordAI : public ScriptedAI
    {
        npc_bloodmaul_magma_lordAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap events;

        void Reset()
        {
            events.Reset();
            events.RescheduleEvent(EVENT_1, 1 * IN_MILLISECONDS);
            events.RescheduleEvent(EVENT_2, urand(3, 6) * IN_MILLISECONDS);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        me->CastSpell(me, SPELL_PILAR_OF_FLAMES);
                        break;
                    case EVENT_2:
                        events.RescheduleEvent(EVENT_2, urand(3, 8) * IN_MILLISECONDS);
                        DoCastVictim(SPELL_FIREBALL);
                        break;
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bloodmaul_magma_lordAI(creature);
    }
};

class npc_bloodmaul_magma_binder : public CreatureScript
{
public:
    npc_bloodmaul_magma_binder() : CreatureScript("npc_bloodmaul_magma_binder") { }

    struct npc_bloodmaul_magma_binderAI : public ScriptedAI
    {
        npc_bloodmaul_magma_binderAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap events;

        void Reset()
        {
            events.Reset();
            events.RescheduleEvent(EVENT_1, urand(2, 5) * IN_MILLISECONDS);
            events.RescheduleEvent(EVENT_2, urand(6, 10) * IN_MILLISECONDS);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        events.RescheduleEvent(EVENT_1, urand(2, 7) * IN_MILLISECONDS);
                        DoCastVictim(SPELL_LAVA_BURST);
                        break;
                    case EVENT_2:
                        events.RescheduleEvent(EVENT_2, urand(10, 16) * IN_MILLISECONDS);
                        me->CastSpell(SelectTarget(SELECT_TARGET_RANDOM), SPELL_MOLTEN_BINDING);
                        break;
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bloodmaul_magma_binderAI(creature);
    }
};

class npc_bloodmaul_slagna : public CreatureScript
{
public:
    npc_bloodmaul_slagna() : CreatureScript("npc_bloodmaul_slagna") { }

    struct npc_bloodmaul_slagnaAI : public ScriptedAI
    {
        npc_bloodmaul_slagnaAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset()
        {
            intro = false;

            me->SetFlag(UNIT_FIELD_FLAGS, 33587264);
            me->SetReactState(REACT_PASSIVE);
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (who->GetTypeId() == TYPEID_PLAYER && !intro)
            {
                intro = true;
                me->SetFlag(UNIT_FIELD_FLAGS, 559168);
                me->SetReactState(REACT_AGGRESSIVE);
                Talk(0);
            }
        }

        void EnterEvadeMode()
        {
            CreatureAI::EnterEvadeMode();
            Reset();
        }

        void UpdateAI(uint32 /*diff*/)
        {
            if (!UpdateVictim())
                return;

            DoSpellAttackIfReady(SPELL_LAVA_SPIT);
        }

    private:
        bool intro;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bloodmaul_slagnaAI(creature);
    }
};

class npc_bloodmaul_molten_earth_elemental : public CreatureScript
{
public:
    npc_bloodmaul_molten_earth_elemental() : CreatureScript("npc_bloodmaul_molten_earth_elemental") { }

    struct npc_bloodmaul_molten_earth_elementalAI : public ScriptedAI
    {
        npc_bloodmaul_molten_earth_elementalAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap events;

        void Reset()
        {
            events.Reset();
            events.RescheduleEvent(EVENT_1, urand(1, 3) * IN_MILLISECONDS);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        events.RescheduleEvent(EVENT_1, urand(3, 7) * IN_MILLISECONDS);
                        DoCastVictim(SPELL_LAVA_ARC);
                        break;
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bloodmaul_molten_earth_elementalAI(creature);
    }
};

class npc_bloodmaul_warder : public CreatureScript
{
public:
    npc_bloodmaul_warder() : CreatureScript("npc_bloodmaul_warder") { }

    struct npc_bloodmaul_warderAI : public ScriptedAI
    {
        npc_bloodmaul_warderAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        void Reset()
        {
            events.Reset();
            events.RescheduleEvent(EVENT_1, urand(1, 3) * IN_MILLISECONDS);
            events.RescheduleEvent(EVENT_2, urand(6, 10) * IN_MILLISECONDS);
        }

        void JustDied(Unit* /*killer*/)
        {
            if (me->FindNearestCreature(BOSS_SLAVE_WATCHER_CRUSHTO, 80.0f, true))
                instance->SetData(DATA_SLAVE_WATCHER_CRUSHTO_EVENT, instance->GetData(DATA_SLAVE_WATCHER_CRUSHTO_EVENT) + 1);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        events.RescheduleEvent(EVENT_1, urand(2, 5) * IN_MILLISECONDS);
                        DoCastVictim(SPELL_CLEAVE);
                        break;
                    case EVENT_2:
                        events.RescheduleEvent(EVENT_2, urand(7, 11) * IN_MILLISECONDS);
                        DoCastVictim(SPELL_FRIGHTENING_ROAR);
                        break;
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }

    private:
        InstanceScript* instance;
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bloodmaul_warderAI(creature);
    }
};

class npc_bloodmaul_orgre_mage : public CreatureScript
{
public:
    npc_bloodmaul_orgre_mage() : CreatureScript("npc_bloodmaul_orgre_mage") { }

    struct npc_bloodmaul_orgre_mageAI : public ScriptedAI
    {
        npc_bloodmaul_orgre_mageAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        void Reset()
        {
            events.Reset();
            events.RescheduleEvent(EVENT_1, urand(1, 4) * IN_MILLISECONDS);
            events.RescheduleEvent(EVENT_2, urand(10, 15) * IN_MILLISECONDS);
        }

        void JustDied(Unit* /*killer*/)
        {
            if (me->FindNearestCreature(BOSS_SLAVE_WATCHER_CRUSHTO, 80.0f, true))
                instance->SetData(DATA_SLAVE_WATCHER_CRUSHTO_EVENT, instance->GetData(DATA_SLAVE_WATCHER_CRUSHTO_EVENT) + 1);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        events.RescheduleEvent(EVENT_1, urand(4, 8) * IN_MILLISECONDS);
                        DoCastVictim(SPELL_LAVA_BURST);
                        break;
                    case EVENT_2:
                        events.RescheduleEvent(EVENT_2, 15 * IN_MILLISECONDS);
                        me->CastSpell(me->SelectNearbyAlly(), SPELL_BLOOD_RAGE);
                        break;
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }

    private:
        InstanceScript* instance;
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bloodmaul_orgre_mageAI(creature);
    }
};

class npc_bloodmaul_overseer : public CreatureScript
{
public:
    npc_bloodmaul_overseer() : CreatureScript("npc_bloodmaul_overseer") { }

    struct npc_bloodmaul_overseerAI : public ScriptedAI
    {
        npc_bloodmaul_overseerAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap events;

        void Reset()
        {
            events.Reset();
            events.RescheduleEvent(EVENT_1, urand(1, 3) * IN_MILLISECONDS);
            events.RescheduleEvent(EVENT_2, urand(7, 12) * IN_MILLISECONDS);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        events.RescheduleEvent(EVENT_1, urand(10, 15) * IN_MILLISECONDS);
                        me->CastSpell(SelectTarget(SELECT_TARGET_RANDOM), SPELL_SUPPRESSION_FIELD);
                        break;
                    case EVENT_2:
                        events.RescheduleEvent(EVENT_2, urand(3, 7) * IN_MILLISECONDS);
                        me->CastSpell(SelectTarget(SELECT_TARGET_RANDOM), SPELL_SUBJUGATE);
                        break;
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bloodmaul_overseerAI(creature);
    }
};

class npc_bloodmaul_enforcer : public CreatureScript
{
public:
    npc_bloodmaul_enforcer() : CreatureScript("npc_bloodmaul_enforcer") { }

    struct npc_bloodmaul_enforcerAI : public ScriptedAI
    {
        npc_bloodmaul_enforcerAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap events;

        void Reset()
        {
            events.Reset();
            events.RescheduleEvent(EVENT_1, urand(1, 3) * IN_MILLISECONDS);
            events.RescheduleEvent(EVENT_2, urand(7, 12) * IN_MILLISECONDS);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        events.RescheduleEvent(EVENT_1, urand(3, 6) * IN_MILLISECONDS);
                        events.RescheduleEvent(EVENT_3, urand(4, 7) * IN_MILLISECONDS);
                        DoCastVictim(SPELL_BEATDOWN);
                        break;
                    case EVENT_3:
                        events.RescheduleEvent(EVENT_1, urand(3, 10) * IN_MILLISECONDS);
                        me->CastSpell(SelectTarget(SELECT_TARGET_RANDOM), SPELL_CRUSH);
                        break;
                    case EVENT_2:
                        events.RescheduleEvent(EVENT_2, urand(7, 12) * IN_MILLISECONDS);
                        me->CastSpell(SelectTarget(SELECT_TARGET_FARTHEST), SPELL_LUMBERING_LEAP);
                        break;
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bloodmaul_enforcerAI(creature);
    }
};

class npc_bloodmaul_slaver : public CreatureScript
{
public:
    npc_bloodmaul_slaver() : CreatureScript("npc_bloodmaul_slaver") { }

    struct npc_bloodmaul_slaverAI : public ScriptedAI
    {
        npc_bloodmaul_slaverAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap events;

        void Reset()
        {
            events.Reset();
            events.RescheduleEvent(EVENT_1, urand(1, 3) * IN_MILLISECONDS);
            events.RescheduleEvent(EVENT_2, urand(5, 12) * IN_MILLISECONDS);
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType)
        {
            if (me->HealthBelowPctDamaged(50, damage))
                me->AddAura(SPELL_SLAVERS_RAGE, me);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        events.RescheduleEvent(EVENT_1, urand(2, 4) * IN_MILLISECONDS);
                        events.RescheduleEvent(EVENT_3, urand(3, 12) * IN_MILLISECONDS);
                        DoCastVictim(SPELL_VICIOUS_SLASH);
                        break;
                    case EVENT_3:
                        me->CastSpell(SelectTarget(SELECT_TARGET_RANDOM), SPELL_SHOCK_BOLA);
                        break;
                    case EVENT_2:
                        events.RescheduleEvent(EVENT_2, urand(5, 8) * IN_MILLISECONDS);
                        me->CastSpell(SelectTarget(SELECT_TARGET_FARTHEST), SPELL_CHAIN_GRIP);
                        break;
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bloodmaul_slaverAI(creature);
    }
};

class npc_bloodmaul_magma_elemental : public CreatureScript
{
public:
    npc_bloodmaul_magma_elemental() : CreatureScript("npc_bloodmaul_magma_elemental") { }

    struct npc_bloodmaul_magma_elementalAI : public ScriptedAI
    {
        npc_bloodmaul_magma_elementalAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap events;
        bool rage;

        void Reset()
        {
            rage = false;
            events.Reset();
            events.RescheduleEvent(EVENT_1, urand(1, 3) * IN_MILLISECONDS);
            events.RescheduleEvent(EVENT_2, urand(5, 12) * IN_MILLISECONDS);
            events.RescheduleEvent(EVENT_3, urand(4, 10) * IN_MILLISECONDS);
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType)
        {
            if (me->HealthBelowPctDamaged(50, damage) && !rage)
            {
                me->AddAura(SPELL_SLAVERS_RAGE, me);
                rage = true;
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        events.RescheduleEvent(EVENT_1, urand(2, 4) * IN_MILLISECONDS);
                        DoCastVictim(SPELL_ARMOR_DENT);
                        break;
                    case EVENT_3:
                        events.RescheduleEvent(EVENT_3, urand(8, 13) * IN_MILLISECONDS);
                        DoCast(SPELL_CINDER_SPLASH);
                        break;
                    case EVENT_2:
                        events.RescheduleEvent(EVENT_2, urand(5, 8) * IN_MILLISECONDS);
                        DoCastVictim(SPELL_FIREBALL_2);
                        break;
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bloodmaul_magma_elementalAI(creature);
    }
};

class npc_bloodmaul_geomancer : public CreatureScript
{
public:
    npc_bloodmaul_geomancer() : CreatureScript("npc_bloodmaul_geomancer") { }

    struct npc_bloodmaul_geomancerAI : public ScriptedAI
    {
        npc_bloodmaul_geomancerAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap events;
        bool rage;

        void Reset()
        {
            rage = false;
            events.Reset();

            events.RescheduleEvent(EVENT_1, urand(1, 3) * IN_MILLISECONDS);
            events.RescheduleEvent(EVENT_2, urand(10, 15) * IN_MILLISECONDS);
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType)
        {
            if (me->HealthBelowPctDamaged(50, damage) && !rage)
            {
                me->AddAura(SPELL_SLAVERS_RAGE, me);
                rage = true;
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        events.RescheduleEvent(EVENT_1, urand(4, 6) * IN_MILLISECONDS);
                        DoCastVictim(SPELL_STONE_BOLT);
                        break;
                    case EVENT_2:
                        events.RescheduleEvent(EVENT_2, urand(20, 30) * IN_MILLISECONDS);
                        if (urand(0, 1))
                            me->CastSpell(me, SPELL_STONE_BULWARK);
                        else
                            me->CastSpell(me->SelectNearbyAlly(), SPELL_STONE_BULWARK);
                        break;
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bloodmaul_geomancerAI(creature);
    }
};

class npc_bloodmaul_croman : public CreatureScript
{
public:
    npc_bloodmaul_croman() : CreatureScript("npc_bloodmaul_croman") { }

    struct npc_bloodmaul_cromanAI : public ScriptedAI
    {
        npc_bloodmaul_cromanAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        InstanceScript* instance;

        void OnSpellClick(Unit* clicker)
        {
            if (clicker->GetTypeId() != TYPEID_PLAYER || instance->GetBossState(DATA_MAGMOLATUS) == DONE)
                return;

            DoCast(151272);
            me->CastSpell(clicker, 163652);
            me->DespawnOrUnsummon(1 * IN_MILLISECONDS);
            instance->SetGuidData(DATA_CROMAN_SUMMONER, clicker->GetGUID());
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bloodmaul_cromanAI(creature);
    }
};

class npc_bloodmaul_croman_warwar : public CreatureScript
{
public:
    npc_bloodmaul_croman_warwar() : CreatureScript("npc_bloodmaul_croman_warwar") { }

    struct npc_bloodmaul_croman_warwarAI : public ScriptedAI
    {
        npc_bloodmaul_croman_warwarAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        void Reset()
        {
            events.Reset();
            events.RescheduleEvent(EVENT_1, 3 * IN_MILLISECONDS);
        }

        void JustDied(Unit* /*killer*/)
        {
            instance->SetData(DATA_CROMAN_PROGRESS, FAIL);
            me->DespawnOrUnsummon(1 * MINUTE * IN_MILLISECONDS);
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
                case ACTION_1:
                    events.RescheduleEvent(EVENT_4, 5 * IN_MILLISECONDS);
                    Talk(8); // 'Я не доверяю ни мужчинам, ни женщинам, ни зверям...'
                    break;
                case ACTION_2:
                    Talk(10); // 'Важно лишь то, что горстка наших выстояла против их орды.'
                    break;
                case ACTION_3:
                    events.RescheduleEvent(EVENT_5, 3 * IN_MILLISECONDS);
                    events.CancelEvent(EVENT_3);
                    Talk(12); // 'Ну наконец-то.'
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        events.RescheduleEvent(EVENT_2, 3 * IN_MILLISECONDS);
                        if (Player* player = ObjectAccessor::GetPlayer(*me, instance->GetGuidData(DATA_CROMAN_SUMMONER)))
                        {
                            me->GetMotionMaster()->MoveFollow(player, 10.0f, MOTION_SLOT_ACTIVE);
                            Talk(0, player->GetGUID()); // 'Ха! Я вижу, храбрости тебе не занимать – но я просто обожаю опасность!'
                            instance->SetData(DATA_CROMAN_PROGRESS, IN_PROGRESS);
                        }
                        break;
                    case EVENT_2:
                        events.RescheduleEvent(EVENT_3, 15 * IN_MILLISECONDS);
                        Talk(1); // 'Что ж... Идём со мной, если хочешь жить.'
                        break;
                    case EVENT_3:
                    {
                        events.RescheduleEvent(EVENT_3, urand(15, 35) * IN_MILLISECONDS);
                        uint32 texts[][2]
                        {
                            {0, 2}, // 'Никто не должен жить в рабстве!'
                            {1, 3}, // 'Давай, сейчас!'
                            {2, 4}, // 'Я твой самый страшный кошмар!'
                            {3, 5}, // 'Если врага можно ранить, значит, его можно и убить!'
                            {4, 6}, // 'Ничто не может причинить мне боль! Только боль.'
                            {5, 7}, // 'Не для меня придет весна.'
                            {6, 8}, // 'Хватит болтать!'
                        };
                        Talk(texts[urand(0, 6)][1]);
                        break;
                    }
                    case EVENT_4:
                        events.RescheduleEvent(EVENT_3, 15 * IN_MILLISECONDS);
                        Talk(9); // 'Но этому я могу довериться.'
                        break;
                    case EVENT_5:
                        Talk(13); // 'Я живу, люблю, убиваю – и совершенно счастлив!'
                        break;
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }

    private:
        InstanceScript* instance;
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bloodmaul_croman_warwarAI(creature);
    }
};

void AddSC_bloodmaul_slag_mines()
{
    new npc_bloodmaul_ogron();
    new npc_bloodmaul_flamespeaker();
    new npc_bloodmaul_gnasher();
    new npc_bloodmaul_magma_lord();
    new npc_bloodmaul_magma_binder();
    new npc_bloodmaul_slagna();
    new npc_bloodmaul_molten_earth_elemental();
    new npc_bloodmaul_warder();
    new npc_bloodmaul_orgre_mage();
    new npc_bloodmaul_overseer();
    new npc_bloodmaul_enforcer();
    new npc_bloodmaul_slaver();
    new npc_bloodmaul_magma_elemental();
    new npc_bloodmaul_geomancer();

    new npc_bloodmaul_croman();
    new npc_bloodmaul_croman_warwar();
}
