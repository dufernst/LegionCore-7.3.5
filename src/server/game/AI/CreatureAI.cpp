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

#include "CreatureAI.h"
#include "CreatureAIImpl.h"
#include "Creature.h"
#include "World.h"
#include "SpellMgr.h"
#include "Vehicle.h"
#include "Log.h"
#include "Player.h"
#include "CreatureTextMgr.h"
#include "Group.h"

bool CreatureAI::UpdateVictimWithGaze()
{
    if (!me->isInCombat())
        return false;

    if (me->HasReactState(REACT_PASSIVE))
    {
        if (me->getVictim())
            return true;
        me->SetReactState(REACT_AGGRESSIVE);
    }

    if (Unit* victim = me->SelectVictim())
        AttackStart(victim);
    return me->getVictim();
}

bool CreatureAI::UpdateVictim()
{
    if (!me->isInCombat() || me->IsDespawn() || !me->IsInWorld())
        return false;

    if (me->GetReactState() == REACT_ATTACK_OFF)
        return false;

    if (!me->HasReactState(REACT_PASSIVE))
    {
        if (Unit* victim = me->SelectVictim())
            AttackStart(victim);
        return me->getVictim();
    }
    if (me->getThreatManager().isThreatListEmpty())
    {
        EnterEvadeMode();
        return false;
    }
    return true;
}

void CreatureAI::SetGazeOn(Unit* target)
{
    if (me->IsValidAttackTarget(target))
    {
        AttackStart(target);
        me->SetReactState(REACT_PASSIVE);
    }
}

bool CreatureAI::IsInDisable()
{
    if (me->HasAuraType(SPELL_AURA_MOD_STUN) || me->HasAuraType(SPELL_AURA_MOD_FEAR) ||
        me->HasAuraType(SPELL_AURA_MOD_CHARM) || me->HasAuraType(SPELL_AURA_MOD_CONFUSE) ||
        me->HasAuraType(SPELL_AURA_MOD_ROOT) || me->HasAuraType(SPELL_AURA_MOD_FEAR_2) ||
        me->HasAuraType(SPELL_AURA_MOD_ROOTED))
        return true;

    return false;
}

bool CreatureAI::IsInControl()
{
    if (me->HasAuraType(SPELL_AURA_MOD_STUN) || me->HasAuraType(SPELL_AURA_MOD_FEAR) ||
        me->HasAuraType(SPELL_AURA_MOD_CHARM) || me->HasAuraType(SPELL_AURA_MOD_CONFUSE) ||
        me->HasAuraType(SPELL_AURA_MOD_FEAR_2) || me->HasAuraType(SPELL_AURA_MOD_PACIFY_SILENCE))
        return true;

    return false;
}

bool CreatureAI::_EnterEvadeMode()
{
    if (!me->isAlive())
        return false;

    // dont remove vehicle auras, passengers arent supposed to drop off the vehicle
    // NPC_HOLY_GUARDIAN or Mindbender or any summons
    switch (me->GetEntry())
    {
    case 46499:
    case 62982:
    case 67236:
    case 35814:
    case 119194:
    case 122183:
        break;
    default:
        if (!me->isAnySummons() && !me->isTrainingDummy())
            me->RemoveAllAurasExceptType(SPELL_AURA_CONTROL_VEHICLE);
        break;
    }

    // sometimes bosses stuck in combat?
    me->DeleteThreatList();
    me->ClearSaveThreatTarget();
    me->CombatStop(true);
    me->LoadCreaturesAddon();
    me->SetLootRecipient(nullptr);
    me->ResetPlayerDamageReq();

    if (me->IsInEvadeMode())
        return false;

    return true;
}

Creature* CreatureAI::DoSummon(uint32 entry, Position const& pos, uint32 despawnTime, TempSummonType summonType)
{
    return me->SummonCreature(entry, pos, summonType, despawnTime);
}

Creature* CreatureAI::DoSummon(uint32 entry, WorldObject* obj, float radius, uint32 despawnTime, TempSummonType summonType)
{
    Position pos;
    obj->GetRandomNearPosition(pos, radius);
    return me->SummonCreature(entry, pos, summonType, despawnTime);
}

Creature* CreatureAI::DoSummonFlyer(uint32 entry, WorldObject* obj, float flightZ, float radius, uint32 despawnTime, TempSummonType summonType)
{
    Position pos;
    obj->GetRandomNearPosition(pos, radius);
    pos.m_positionZ += flightZ;
    return me->SummonCreature(entry, pos, summonType, despawnTime);
}

AISpellInfoType* UnitAI::AISpellInfo;

//Disable CreatureAI when charmed
void CreatureAI::OnCharmed(bool /*apply*/)
{
    //me->IsAIEnabled = !apply;*/
    me->NeedChangeAI = true;
    me->IsAIEnabled = false;
}

CreatureAI::CreatureAI(Creature* creature) : UnitAI(creature), me(creature), m_MoveInLineOfSight_locked(false), m_canSeeEvenInPassiveMode(false), inFightAggroCheck_Timer(TIME_INTERVAL_LOOK)
{
}

void CreatureAI::Talk(std::initializer_list<uint8> ids, ObjectGuid WhisperGuid /*= ObjectGuid::Empty*/)
{
    sCreatureTextMgr->SendChat(me, Trinity::Containers::SelectRandomContainerElement(ids), WhisperGuid);
}

void CreatureAI::Talk(uint8 id, ObjectGuid WhisperGuid)
{
    if (!this)
        return;

    sCreatureTextMgr->SendChat(me, id, WhisperGuid);
}

void CreatureAI::TalkAuto(uint8 id, ObjectGuid WhisperGuid)
{
    if (!this)
        return;

    sCreatureTextMgr->SendChat(me, id, WhisperGuid, CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_NORMAL, 0, TEAM_OTHER, false, nullptr, true);
}

void CreatureAI::DelayTalk(uint32 delayTimer, uint8 id, ObjectGuid WhisperGuid)
{
    if (!this)
        return;

    delayTimer *= IN_MILLISECONDS;

    me->AddDelayedEvent(delayTimer, [this, id, WhisperGuid]() -> void
    {
        if (me && me->isAlive())
            Talk(id, WhisperGuid);
    });
}

void CreatureAI::ZoneTalk(uint8 id, ObjectGuid WhisperGuid)
{
    if (!this)
        return;

    sCreatureTextMgr->SendChat(me, id, WhisperGuid, CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_ZONE);
}

void CreatureAI::DoZoneInCombat(Creature* creature /*= NULL*/, float maxRangeToNearestTarget /* = 50.0f*/)
{
    if (!creature)
        creature = me;

    if (!creature->CanHaveThreatList())
        return;

    Map* map = creature->GetMap();
    if (!map->IsDungeon())                                  //use IsDungeon instead of Instanceable, in case battlegrounds will be instantiated
    {
        TC_LOG_DEBUG(LOG_FILTER_GENERAL, "DoZoneInCombat call for map that isn't an instance (creature entry = %d)", creature->IsCreature() ? creature->ToCreature()->GetEntry() : 0);
        return;
    }

    if (!creature->HasReactState(REACT_PASSIVE) && !creature->getVictim())
    {
        if (Unit* nearTarget = creature->SelectNearestTarget(maxRangeToNearestTarget))
            creature->AI()->AttackStart(nearTarget);
        else if (creature->isSummon())
        {
            if (Unit* summoner = creature->ToTempSummon()->GetSummoner())
            {
                Unit* target = summoner->getAttackerForHelper();
                if (!target && summoner->CanHaveThreatList() && !summoner->getThreatManager().isThreatListEmpty())
                    target = summoner->getThreatManager().getHostilTarget();
                if (target && (creature->IsFriendlyTo(summoner) || creature->IsHostileTo(target)))
                    creature->AI()->AttackStart(target);
            }
        }
    }

    if (!creature->HasReactState(REACT_PASSIVE) && !creature->getVictim())
    {
        TC_LOG_DEBUG(LOG_FILTER_GENERAL, "DoZoneInCombat called for creature that has empty threat list (creature entry = %u)", creature->GetEntry());
        return;
    }

    map->ApplyOnEveryPlayer([&](Player* player)
    {
        if (player->isGameMaster())
            return;

        if (player->isAlive())
        {
            creature->SetInCombatWith(player);
            player->SetInCombatWith(creature);
            creature->AddThreat(player, 0.0f);
        }

        /* Causes certain things to never leave the threat list (Priest Lightwell, etc):
        for (Unit::ControlList::const_iterator itr = player->m_Controlled.begin(); itr != player->m_Controlled.end(); ++itr)
        {
        creature->SetInCombatWith(*itr);
        (*itr)->SetInCombatWith(creature);
        creature->AddThreat(*itr, 0.0f);
        }*/
    });
}

void CreatureAI::DoAttackerAreaInCombat(Unit* attacker, float range, Unit* pUnit)
{
    if (!attacker)
        attacker = me;

    if (!pUnit)
        pUnit = me;

    Map *map = pUnit->GetMap();
    if (!map->IsDungeon())
        return;

    if (!pUnit->CanHaveThreatList() || pUnit->getThreatManager().isThreatListEmpty())
        return;

    map->ApplyOnEveryPlayer([&](Player* player)
    {
        if (player->isAlive() && attacker->GetDistance(player) <= range)
        {
            pUnit->SetInCombatWith(player);
            player->SetInCombatWith(pUnit);
            pUnit->AddThreat(player, 0.0f);
        }
    });
}

void CreatureAI::DoAttackerGroupInCombat(Player* attacker)
{
    if (attacker)
    {
        if (Group* group = attacker->GetGroup())
        {
            for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
            {
                Player* player = itr->getSource();

                if (player && player->isAlive() && !player->isInCombat() && player->GetMapId() == me->GetMapId())
                {
                    me->SetInCombatWith(player);
                    player->SetInCombatWith(me);
                    me->AddThreat(player, 0.0f);
                }
            }
        }
    }
}

void CreatureAI::DoAggroPulse(uint32 diff)
{
    if (inFightAggroCheck_Timer <= diff)
    {
        std::list<HostileReference*> t_list = me->getThreatManager().getThreatList();
        if (!t_list.empty())
        {
            for (std::list<HostileReference*>::const_iterator itr = t_list.begin(); itr != t_list.end(); ++itr)
            {
                if (auto player = Player::GetPlayer(*me, (*itr)->getUnitGuid()))
                {
                    if (player->isAlive())
                    {
                        me->SetInCombatWithZone();
                        break;
                    }
                }
            }
        }
        inFightAggroCheck_Timer = TIME_INTERVAL_LOOK;
    }
    else
        inFightAggroCheck_Timer -= diff;
}

// scripts does not take care about MoveInLineOfSight loops
// MoveInLineOfSight can be called inside another MoveInLineOfSight and cause stack overflow
void CreatureAI::MoveInLineOfSight_Safe(Unit* who)
{
    if (m_MoveInLineOfSight_locked)
        return;

    m_MoveInLineOfSight_locked = true;
    MoveInLineOfSight(who);
    m_MoveInLineOfSight_locked = false;
}

bool CreatureAI::CanSeeEvenInPassiveMode()
{
    return m_canSeeEvenInPassiveMode;
}

void CreatureAI::SetCanSeeEvenInPassiveMode(bool canSeeEvenInPassiveMode)
{
    m_canSeeEvenInPassiveMode = canSeeEvenInPassiveMode;
}

void CreatureAI::AddDelayedEvent(uint64 timeOffset, std::function<void()>&& function)
{
    me->AddDelayedEvent(timeOffset, std::move(function));
}

void CreatureAI::KillAllDelayedEvents()
{
    me->KillAllDelayedEvents();
}

void CreatureAI::AddDelayedCombat(uint64 timeOffset, std::function<void()>&& function)
{
    me->AddDelayedCombat(timeOffset, std::move(function));
}

void CreatureAI::KillAllDelayedCombats()
{
    me->KillAllDelayedCombats();
}

void CreatureAI::MoveInLineOfSight(Unit* who)
{
    if (me->getVictim())
        return;

    if (me->GetCreatureType() == CREATURE_TYPE_NON_COMBAT_PET) // non-combat pets should just stand there and look good;)
        return;

    if (me->canStartAttack(who, false))
        AttackStart(who);
}

void CreatureAI::EnterEvadeMode()
{
    if (!_EnterEvadeMode())
        return;

    TC_LOG_DEBUG(LOG_FILTER_UNITS, "Creature %u enters evade mode.", me->GetEntry());

    if (!me->GetVehicle()) // otherwise me will be in evade mode forever
    {
        if (Unit* owner = me->GetCharmerOrOwner())
        {
            me->GetMotionMaster()->Clear(false);
            me->GetMotionMaster()->MoveFollow(owner, me->GetFollowDistance(), me->GetFollowAngle(), MOTION_SLOT_ACTIVE);
        }
        else
        {
            if (!me->HasUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED) || !me->isInCombat())
                me->AddUnitState(UNIT_STATE_EVADE);

            me->GetMotionMaster()->MoveTargetedHome();
        }
    }

    if (!me->IsDespawn())
    {
        me->m_Events.KillAllEvents(true);

        if (me->IsVisible())
            me->KillAllDelayedCombats();
    }

    Reset();

    if (me->IsVehicle()) // use the same sequence of addtoworld, aireset may remove all summons!
        me->GetVehicleKit()->Reset(true);
}

void CreatureAI::DespawnOnRespawn(uint32 uiTimeToDespawn)
{
    me->AddDelayedEvent(100, [this, uiTimeToDespawn]() -> void
    {
        me->DespawnOrUnsummon();
        me->SetRespawnTime(uiTimeToDespawn);
    });
}

void CreatureAI::SetFlyMode(bool fly)
{
    me->SetCanFly(fly);
    me->SetDisableGravity(fly);
    if (fly)
        me->SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_HOVER);
    else
        me->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_HOVER);
}

AISpellInfoType::AISpellInfoType() : target(AITARGET_SELF), condition(AICOND_COMBAT)
, cooldown(AI_DEFAULT_COOLDOWN), realCooldown(0), maxRange(0.0f)
{
}

AISpellInfoType* GetAISpellInfo(uint32 i) { return &CreatureAI::AISpellInfo[i]; }
