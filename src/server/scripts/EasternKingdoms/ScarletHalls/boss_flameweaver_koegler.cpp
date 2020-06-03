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

#include "ScriptedEscortAI.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "scarlet_halls.h"

enum Spells
{
    SPELL_COSMETIC_FLAME        = 126645,
    SPELL_PYROBLAST             = 113690,
    SPELL_QUICKENED_MIND        = 113682,
    SPELL_FIREBALL_VOLLEY       = 113691,
    SPELL_TELEPORT              = 113626,
    SPELL_DRAGON_BREATH         = 113641,
    SPELL_BOOK_BURNER           = 113366,
    SPELL_BURNING_BOOKS         = 113616,
};

enum Says
{
    SAY_ENTER_PLAYER    = 0,
    SAY_AGGRO           = 1,
    SAY_BOOK_BURN       = 2,
    SAY_BREATH_WARNING  = 3,
    SAY_BREATH          = 4,
    SAY_DEATH           = 5,
};

enum eEvants
{
    ACTION_INTRO            = 1,
    
    EVENT_INTRO             = 2,
    EVENT_PYROBLAST         = 3,
    EVENT_QUICKENED_MIND    = 4,
    EVENT_FIREBALL_VOLLEY   = 5,
    EVENT_TELEPORT          = 6,
    EVENT_BREATH_START      = 7,
    EVENT_BREATH_FINISH     = 8,
    EVENT_BOOK_BURNER       = 9,
};

class boss_flameweaver_koegler : public CreatureScript
{
public:
    boss_flameweaver_koegler() : CreatureScript("boss_flameweaver_koegler") {}

    struct boss_flameweaver_koeglerAI : public BossAI
    {
        boss_flameweaver_koeglerAI(Creature* creature) : BossAI(creature, DATA_KOEGLER) 
        {
            instance = me->GetInstanceScript();
            intro = true;
        }

        InstanceScript* instance;
        EventMap events;

        uint32 Check_Timer;

        bool Orient;
        bool intro;

        void Reset()
        {
            Orient = false;
            Check_Timer = 500;
            events.Reset();
            events.RescheduleEvent(EVENT_INTRO, 1000);
            summons.DespawnAll();
            me->RemoveAllAuras();
            me->SetReactState(REACT_AGGRESSIVE);
            _Reset();
        }

        void EnterCombat(Unit* /*who*/) 
        {
            Talk(SAY_AGGRO);
            events.CancelEvent(EVENT_INTRO);
            events.RescheduleEvent(EVENT_PYROBLAST, 4000);
            events.RescheduleEvent(EVENT_QUICKENED_MIND, 8000);
            events.RescheduleEvent(EVENT_FIREBALL_VOLLEY, 14000);
            events.RescheduleEvent(EVENT_TELEPORT, 30000);
            events.RescheduleEvent(EVENT_BOOK_BURNER, 22000);
            _EnterCombat();
        }

        void JustDied(Unit* /*killer*/)
        {
            _JustDied();
        }

        void DoAction(const int32 action)
        {
            if (action == ACTION_INTRO && intro)
            {
                intro = false;
                Talk(SAY_ENTER_PLAYER);
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim() && me->isInCombat())
                return;

            if (me->GetDistance(me->GetHomePosition()) > 30.0f)
            {
                EnterEvadeMode();
                return;
            }

            events.Update(diff);
            
            /*if (Orient)
            {
                if (Check_Timer <= diff)
                {
                    if (Unit* target = me->FindNearestCreature(NPC_DRAGON_BREATH_TARGET, 30.0f))
                        me->SetFacingToObject(target);

                    Check_Timer = 500;
                }
                else Check_Timer -= diff;
            }*/

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_INTRO:
                        DoCast(SPELL_COSMETIC_FLAME);
                        events.RescheduleEvent(EVENT_INTRO, 5000);
                        break;
                    case EVENT_PYROBLAST:
                        if (Unit* target = me->getVictim())
                            DoCast(target, SPELL_PYROBLAST);
                        events.RescheduleEvent(EVENT_PYROBLAST, 1000, 0);
                        break;
                    case EVENT_QUICKENED_MIND:
                        DoCast(SPELL_QUICKENED_MIND);
                        events.RescheduleEvent(EVENT_QUICKENED_MIND, 30000);
                        break;
                    case EVENT_FIREBALL_VOLLEY:
                        DoCast(SPELL_FIREBALL_VOLLEY);
                        events.RescheduleEvent(EVENT_FIREBALL_VOLLEY, 30000);
                        break;
                    case EVENT_TELEPORT:
                        me->SetReactState(REACT_PASSIVE);
                        me->AttackStop();
                        me->SummonCreature(NPC_DRAGON_BREATH_TARGET, 1279.58f, 549.58f, 12.90f);
                        DoCast(SPELL_TELEPORT);
                        Talk(SAY_BREATH);
                        events.RescheduleEvent(EVENT_TELEPORT, 46000);
                        events.RescheduleEvent(EVENT_BREATH_START, 1000);
                        break;
                    case EVENT_BREATH_START:
                        Talk(SAY_BREATH_WARNING);
                        Orient = true;
                        DoCast(SPELL_DRAGON_BREATH);
                        events.RescheduleEvent(EVENT_BREATH_FINISH, 1000);
                        break;
                    case EVENT_BREATH_FINISH:
                        Orient = false;
                        me->SetReactState(REACT_AGGRESSIVE);
                        break;
                    case EVENT_BOOK_BURNER:
                        Talk(SAY_BOOK_BURN);
                        DoCast(SPELL_BOOK_BURNER);
                        events.RescheduleEvent(EVENT_BOOK_BURNER, 32000);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };
    
    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_flameweaver_koeglerAI (creature);
    }
};

class npc_dragon_breath_target : public CreatureScript
{
public:
    npc_dragon_breath_target() : CreatureScript("npc_dragon_breath_target") {}

    struct npc_dragon_breath_targetAI : public npc_escortAI
    {
        npc_dragon_breath_targetAI(Creature* creature) : npc_escortAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            InitWaypoint();
        }

        InstanceScript* instance;

        void InitWaypoint()
        {
            AddWaypoint(0, 1284.60f, 563.04f, 13.85f);
            AddWaypoint(1, 1298.03f, 568.60f, 13.85f);
            AddWaypoint(2, 1311.47f, 563.04f, 13.85f);
            AddWaypoint(3, 1317.03f, 549.60f, 13.85f);
            AddWaypoint(4, 1311.47f, 536.17f, 13.85f);
            AddWaypoint(5, 1298.03f, 530.60f, 13.85f);
            AddWaypoint(6, 1284.60f, 536.17f, 13.85f);
            AddWaypoint(7, 1279.03f, 549.60f, 13.85f);
        }

        void SpellHit(Unit* attacker, const SpellInfo* spell)
        {
            if (!instance)
                return;

            if (spell->Id == SPELL_DRAGON_BREATH)
            {
                Start(false, false);
                SetWPStarTimer(500);
            }
        }
        
        void WaypointReached(uint32 id) 
        {
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_dragon_breath_targetAI(creature);
    }
};

class npc_book_case : public CreatureScript
{
public:
    npc_book_case() : CreatureScript("npc_book_case") {}

    struct npc_book_caseAI : public Scripted_NoMovementAI
    {
        npc_book_caseAI(Creature* creature) : Scripted_NoMovementAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;

        void SpellHit(Unit* attacker, const SpellInfo* spell)
        {
            if (!instance)
                return;

            if (spell->Id == SPELL_BOOK_BURNER)
                me->CastSpell(me, SPELL_BURNING_BOOKS, true);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_book_caseAI(creature);
    }
};

class at_koegler_enter_room : public AreaTriggerScript
{
public:
    at_koegler_enter_room() : AreaTriggerScript("at_koegler_enter_room") { }

    bool OnTrigger(Player* pPlayer, const AreaTriggerEntry* /*pAt*/, bool enter)
    {
        if (pPlayer->isGameMaster())
            return false;
        
        if (InstanceScript* instance = pPlayer->GetInstanceScript())
        {
            if (Creature* koegler = instance->instance->GetCreature(instance->GetGuidData(NPC_FLAMEWEAVER_KOEGLER)))
                koegler->AI()->DoAction(ACTION_INTRO);
        }
        return true;
    }
};

void AddSC_boss_flameweaver_koegler()
{
    new boss_flameweaver_koegler();
    new npc_dragon_breath_target();
    new npc_book_case();
    new at_koegler_enter_room();
}
