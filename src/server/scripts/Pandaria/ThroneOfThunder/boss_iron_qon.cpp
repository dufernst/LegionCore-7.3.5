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

#include "throne_of_thunder.h"

enum eSpells
{
    //Special
    SPELL_RIDE_VEHICLE      = 46598,
    SPELL_ANIM_SIT          = 128886,
    //Iron Qon
    SPELL_THROW_SPEAR       = 134925, 
    SPELL_IMPALE            = 134691, 
    SPELL_RISING_ANGER      = 136324,
    SPELL_FIST_SMASH        = 136146,

    SPELL_BURNING_CINDERS   = 137668,
    SPELL_FROSEN_BLOOD      = 137664,
    SPELL_STORM_CLOUD       = 137669,
    SPELL_FROSEN_SOLID      = 136892,
    SPELL_RUSHING_WINDS     = 135583,
};

const uint32 mauntEntry[3] = 
{
    NPC_ROSHAK,
    NPC_QUETZAL,
    NPC_DAMREN,
};

enum sEvents
{
    EVENT_THROW_SPEAR       = 1,
    EVENT_IMPALE            = 2,
    EVENT_FIST_SMASH        = 3,
};

class boss_iron_qon : public CreatureScript
{
    public:
        boss_iron_qon() : CreatureScript("boss_iron_qon") {}

        struct boss_iron_qonAI : public BossAI
        {
            boss_iron_qonAI(Creature* creature) : BossAI(creature, DATA_IRON_QON)
            {
                instance = creature->GetInstanceScript();
            }

            InstanceScript* instance;
            bool achieve;

            void Reset()
            {
                _Reset();
                ResetMaunt();
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
            }

            void SpellHitTarget(Unit* target, const SpellInfo* spell) override
            {
                switch (spell->Id)
                {
                case SPELL_BURNING_CINDERS:
                case SPELL_FROSEN_BLOOD:
                case SPELL_STORM_CLOUD:
                case SPELL_FROSEN_SOLID:
                case SPELL_RUSHING_WINDS:
                    achieve = false;
                    break;
                }
            }

            void ResetMaunt()
            {
                if (instance)
                {
                    for (uint8 n = 0; n < 3; n++)
                    {
                        if (Creature* maunt = me->GetCreature(*me, instance->GetGuidData(mauntEntry[n])))
                        {
                            if (!maunt->isAlive())
                            {
                                maunt->Respawn();
                                maunt->GetMotionMaster()->MoveTargetedHome();
                            }
                        }
                    }
                }
            }

            void EnterCombat(Unit* who)
            {
                me->AddAura(SPELL_RISING_ANGER, me);
                events.RescheduleEvent(EVENT_FIST_SMASH, 15000);
                achieve = true;
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                if (achieve)
                {
                    DoCast(me, 139065, true);
                    DoCast(me, 139064, true);
                    DoCast(me, 139061, true);
                    DoCast(me, 139062, true);
                    DoCast(me, 139063, true);
                }
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    if (eventId == EVENT_FIST_SMASH)
                    {
                        DoCast(me, SPELL_FIST_SMASH);
                        events.RescheduleEvent(EVENT_FIST_SMASH, 11000);
                    }           
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_iron_qonAI(creature);
        }
};

class npc_iron_qon_maunt : public CreatureScript
{
    public:
        npc_iron_qon_maunt() : CreatureScript("npc_iron_qon_maunt") {}

        struct npc_iron_qon_mauntAI : public ScriptedAI
        {
            npc_iron_qon_mauntAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
                spawnvehtimer = 0;
                if (me->GetEntry() != NPC_ROSHAK)
                    DoCast(me, SPELL_ANIM_SIT, true);
                else
                    spawnvehtimer = 2000;
            }

            InstanceScript* instance;
            EventMap events;
            uint32 spawnvehtimer;

            void Reset()
            {
                events.Reset();
                if (instance)
                {
                    CheckDiedMaunts();
                    if (me->HasAura(SPELL_RIDE_VEHICLE))
                        RemovePassenger();
                    if (me->GetEntry() == NPC_ROSHAK)
                    {
                        FlyControl(me, true);
                        me->SetReactState(REACT_AGGRESSIVE);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                    }
                    else
                    {
                        FlyControl(me, false);
                        me->SetReactState(REACT_PASSIVE);
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                    }
                    instance->SetBossState(DATA_IRON_QON, NOT_STARTED);
                }
            }

            void FlyControl(Creature* maunt, bool state)
            {
                if (maunt)
                {
                    if (state)
                    {
                        maunt->SetCanFly(true);
                        maunt->SetDisableGravity(true);
                        maunt->SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
                    }
                    else
                    {
                        maunt->SetCanFly(false);
                        maunt->SetDisableGravity(false);
                        maunt->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
                    }
                }
            }

            void CheckDiedMaunts()
            {
                if (instance)
                {
                    for (uint8 n = 0; n < 3; n++)
                    {
                        if (Creature* maunt = me->GetCreature(*me, instance->GetGuidData(mauntEntry[n])))
                        {
                            if (me->GetEntry() != mauntEntry[n] && !maunt->isAlive())
                            {
                                maunt->Respawn();
                                maunt->GetMotionMaster()->MoveTargetedHome();
                            }
                        }
                    }
                }
            }

            void JustReachedHome()
            {
                if (me->GetEntry() != NPC_ROSHAK)
                    DoCast(me, SPELL_ANIM_SIT, true);
                else
                {
                    if (Creature* iq = me->GetCreature(*me, instance->GetGuidData(NPC_IRON_QON)))
                        iq->CastSpell(me, SPELL_RIDE_VEHICLE);
                }
            }

            void RemovePassenger()
            {
                 if (Creature* iq = me->GetCreature(*me, instance->GetGuidData(NPC_IRON_QON)))
                 {
                     if (iq->isAlive())
                         iq->RemoveAurasDueToSpell(SPELL_RIDE_VEHICLE);
                 }
            }

            void MovementInform(uint32 type, uint32 pointId)
            {
                if (type == POINT_MOTION_TYPE)
                {
                    if (pointId == 0)
                    {
                       if (Creature* iq = me->GetCreature(*me, instance->GetGuidData(NPC_IRON_QON)))
                       {
                           iq->CastSpell(me, SPELL_RIDE_VEHICLE);
                           me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                           if (Player* pl = me->FindNearestPlayer(20.0f, true))
                           {
                               if (!pl->isGameMaster())
                               {
                                   me->SetReactState(REACT_AGGRESSIVE);
                                   DoZoneInCombat(me, 100.0f);
                                   return;
                               }
                           }
                           EnterEvadeMode();
                       }
                    }
                }
            }

            void EnterCombat(Unit* who)
            {
                if (instance && me->GetEntry() == NPC_ROSHAK) //First maunt
                {
                    instance->SetBossState(DATA_IRON_QON, IN_PROGRESS);
                    DoZoneInCombat(me, 100.0f);
                }
                events.RescheduleEvent(EVENT_THROW_SPEAR, 30000);
                events.RescheduleEvent(EVENT_IMPALE,      20000);
            }

            void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
            {
                if (damage >= me->GetHealth())
                {
                    FlyControl(me, false);
                    FindAndUseNextMaunt();
                }
            }

            void FindAndUseNextMaunt()
            {
                if (instance)
                {
                    if (Creature* iq = me->GetCreature(*me, instance->GetGuidData(NPC_IRON_QON)))
                    {
                        iq->RemoveAurasDueToSpell(SPELL_RIDE_VEHICLE);

                        switch (me->GetEntry())
                        {
                        case NPC_ROSHAK:
                            if (Creature* q = me->GetCreature(*me, instance->GetGuidData(NPC_QUETZAL)))
                            {
                                if (q->isAlive())
                                {
                                    q->RemoveAurasDueToSpell(SPELL_ANIM_SIT);
                                    FlyControl(q, true);
                                    q->GetMotionMaster()->MoveCharge(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 15.0f, 0);
                                }
                            }
                            break;
                        case NPC_QUETZAL:
                            if (Creature* d = me->GetCreature(*me, instance->GetGuidData(NPC_DAMREN)))
                            {
                                if (d->isAlive())
                                {
                                    d->RemoveAurasDueToSpell(SPELL_ANIM_SIT);
                                    FlyControl(d, true);
                                    d->GetMotionMaster()->MoveCharge(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 15.0f, 0);
                                }
                            }
                            break;
                        case NPC_DAMREN:
                            iq->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                            iq->SetReactState(REACT_AGGRESSIVE);
                            iq->AI()->DoZoneInCombat(iq, 100.0f);
                            break;
                        }
                    }
                }
            }

            void UpdateAI(uint32 diff)
            {
                if (spawnvehtimer)
                {
                    if (spawnvehtimer <= diff)
                    {
                        spawnvehtimer = 0;
                        if (Creature* iq = me->FindNearestCreature(NPC_IRON_QON, 100.0f, true))
                            iq->CastSpell(me, SPELL_RIDE_VEHICLE);
                    }
                    else
                        spawnvehtimer -= diff;
                }

                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_THROW_SPEAR:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 40.0f, true))
                            DoCast(target, SPELL_THROW_SPEAR);
                        events.RescheduleEvent(EVENT_THROW_SPEAR, 30000);
                        break;
                    case EVENT_IMPALE:
                        if (me->getVictim())
                            DoCast(me->getVictim(), SPELL_IMPALE);
                        events.RescheduleEvent(EVENT_IMPALE, 20000);
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_iron_qon_mauntAI(creature);
        }
};

void AddSC_boss_iron_qon()
{
    new boss_iron_qon();
    new npc_iron_qon_maunt();
}
