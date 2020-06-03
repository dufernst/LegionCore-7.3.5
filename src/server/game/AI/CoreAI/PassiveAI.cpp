/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#include "PassiveAI.h"
#include "Creature.h"

PassiveAI::PassiveAI(Creature* c) : CreatureAI(c) { me->SetReactState(REACT_PASSIVE); }
PossessedAI::PossessedAI(Creature* c) : CreatureAI(c) { me->SetReactState(REACT_PASSIVE); }
NullCreatureAI::NullCreatureAI(Creature* c) : CreatureAI(c) { me->SetReactState(REACT_PASSIVE); }

void PassiveAI::UpdateAI(uint32)
{
    if (me->isInCombat() && me->getAttackers()->empty())
        EnterEvadeMode();
}

void PossessedAI::AttackStart(Unit* target)
{
    me->Attack(target, true);
}

void PossessedAI::UpdateAI(uint32 /*diff*/)
{
    if (me->getVictim())
    {
        if (!me->IsValidAttackTarget(me->getVictim()))
            me->AttackStop();
        else
            DoMeleeAttackIfReady();
    }
}

void PossessedAI::JustDied(Unit* /*u*/)
{
    // We died while possessed, disable our loot
    me->RemoveFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
}

void PossessedAI::KilledUnit(Unit* victim)
{
    // We killed a creature, disable victim's loot
    if (victim->IsCreature())
        victim->RemoveFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
}

void CritterAI::InitializeAI()
{
    if (me->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
        return;

    me->AddDelayedEvent(2000, [this]
    {
        auto movementType = me->GetDefaultMovementType();
        if (movementType < MAX_DB_MOTION_TYPE && movementType != WAYPOINT_MOTION_TYPE)
            me->GetMotionMaster()->MoveRandom(urand(30, 50));
    });
}

void CritterAI::EnterCombat(Unit * who)
{
    if (!me->HasUnitState(UNIT_STATE_FLEEING))
        me->GetMotionMaster()->MoveFleeing(who);
}

void CritterAI::DamageTaken(Unit* /*done_by*/, uint32&, DamageEffectType /*dmgType*/)
{
    if (!me->HasUnitState(UNIT_STATE_FLEEING))
        me->SetControlled(true, UNIT_STATE_FLEEING);
}

void CritterAI::EnterEvadeMode()
{
    if (me->HasUnitState(UNIT_STATE_FLEEING))
        me->SetControlled(false, UNIT_STATE_FLEEING);
    CreatureAI::EnterEvadeMode();
}

void CritterAI::AttackedBy(Unit * who)
{
    EnterCombat(who);
}

void TriggerAI::IsSummonedBy(Unit* summoner)
{
    if (me->m_templateSpells[0])
        me->CastSpell(me, me->m_templateSpells[0], false, nullptr, nullptr, summoner->GetGUID());
}
