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

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "arena_of_annihilation.h"

enum Spells
{
    SPELL_STONE_SPIN        = 123928,
    SPELL_CRUSHING_BITE     = 123918,
};

enum Events
{
    EVENT_POINT_HOME        = 1,
    EVENT_STONE_SPIN        = 2,
    EVENT_STONE_SPIN2       = 3,
    EVENT_STONE_SPIN3       = 4,
    EVENT_CRUSHING_BITE     = 5,
};

const Position pos[11] = 
{
    {3828.93f, 559.35f, 639.69f},
    {3820.97f, 499.60f, 639.69f},
    {3841.55f, 527.54f, 639.69f},
    {3784.21f, 535.31f, 639.69f},
    {3754.85f, 564.94f, 639.69f},
    {3808.40f, 532.22f, 639.69f},
    {3791.59f, 485.88f, 639.69f},
    {3793.75f, 521.58f, 639.69f},
    {3758.01f, 511.44f, 639.69f},
    {3797.27f, 545.71f, 639.69f},
    {3801.23f, 578.68f, 639.69f},
};

class boss_scar_shell : public CreatureScript
{
public:
    boss_scar_shell() : CreatureScript("boss_scar_shell") { }

    struct boss_scar_shellAI : public BossAI
    {
        boss_scar_shellAI(Creature* creature) : BossAI(creature, DATA_SCAR_SHELL)
        {
            instance = me->GetInstanceScript();
        }

        InstanceScript* instance;

        void Reset() override
        {
            events.Reset();
            me->SetReactState(REACT_PASSIVE);
            events.RescheduleEvent(EVENT_POINT_HOME, 2000);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            _EnterCombat();
            events.RescheduleEvent(EVENT_STONE_SPIN, 16000);
            events.RescheduleEvent(EVENT_CRUSHING_BITE, 9000);
        }

        void EnterEvadeMode() override
        {
            _Reset();
            me->DespawnOrUnsummon();
        }

        void JustDied(Unit* /*killer*/) override
        {
            _JustDied();
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type == POINT_MOTION_TYPE && id != 1)
                events.RescheduleEvent(EVENT_STONE_SPIN2, 0);

            else if (type == POINT_MOTION_TYPE && id == 1)
            {
                if (GameObject* pGo = instance->instance->GetGameObject(instance->GetGuidData(DATA_DOOR)))
                    pGo->SetGoState(GO_STATE_READY);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                me->SetReactState(REACT_AGGRESSIVE);
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim() && me->isInCombat())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_POINT_HOME:
                        if (GameObject* pGo = instance->instance->GetGameObject(instance->GetGuidData(DATA_DOOR)))
                            pGo->SetGoState(GO_STATE_ACTIVE);
                        me->GetMotionMaster()->MovePoint(1, centerPos);
                        break;
                    case EVENT_STONE_SPIN:
                        me->StopAttack();
                        DoCast(SPELL_STONE_SPIN);
                        events.RescheduleEvent(EVENT_STONE_SPIN, 50000);
                        events.RescheduleEvent(EVENT_STONE_SPIN2, 2000);
                        events.RescheduleEvent(EVENT_STONE_SPIN3, 14000);
                        break;
                    case EVENT_STONE_SPIN2:
                    {
                        uint8 i = urand(0, 10);
                        me->GetMotionMaster()->MovePoint(2, pos[i]);
                        break;
                    }
                    case EVENT_STONE_SPIN3:
                        events.CancelEvent(EVENT_STONE_SPIN2);
                        me->SetReactState(REACT_AGGRESSIVE);
                        break;
                    case EVENT_CRUSHING_BITE:
                        if (Unit* target = me->getVictim())
                            DoCast(target, SPELL_CRUSHING_BITE);
                        events.RescheduleEvent(EVENT_CRUSHING_BITE, 12000);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_scar_shellAI (creature);
    }
};

void AddSC_boss_scar_shell()
{
    new boss_scar_shell();
}
