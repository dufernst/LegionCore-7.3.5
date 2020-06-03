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

#include "UnitAI.h"
#include "Creature.h"
#include "SpellAuras.h"
#include "SpellAuraEffects.h"
#include "SpellMgr.h"
#include "SpellInfo.h"
#include "Spell.h"
#include "CreatureAIImpl.h"

void UnitAI::AttackStart(Unit* victim)
{
    if (victim && me->Attack(victim, true))
        me->GetMotionMaster()->MoveChase(victim);
}

void UnitAI::AttackStartCaster(Unit* victim, float dist)
{
    if (victim && me->Attack(victim, false))
        me->GetMotionMaster()->MoveChase(victim, dist);
}

bool UnitAI::DoMeleeAttackIfReady(SpellSchoolMask schoolMask)
{
    if (me->HasUnitState(UNIT_STATE_CASTING))
        return false;

    if (me->IsCreature() && me->ToCreature()->GetReactState() == REACT_PASSIVE && !me->GetGuidValue(UNIT_FIELD_TARGET))
        return false;

    bool attackSuccess = false;

    Unit* victim = me->getVictim();
    //Make sure our attack is ready and we aren't currently casting before checking distance
    if (me->isAttackReady() && me->IsWithinMeleeRange(victim))
    {
        me->AttackerStateUpdate(victim, BASE_ATTACK, false, me->GetAutoattackSpellId(BASE_ATTACK), 0, schoolMask);
        me->resetAttackTimer();
        attackSuccess = true;
    }

    if (me->haveOffhandWeapon() && me->isAttackReady(OFF_ATTACK) && me->IsWithinMeleeRange(victim))
    {
        me->AttackerStateUpdate(victim, OFF_ATTACK, false, me->GetAutoattackSpellId(OFF_ATTACK), 0, schoolMask);
        me->resetAttackTimer(OFF_ATTACK);
        attackSuccess = true;
    }

    return attackSuccess;
}

bool UnitAI::DoSpellAttackIfReady(uint32 spell, TriggerCastFlags triggerFlags /*= TRIGGERED_NONE*/, Unit* target /*= nullptr*/, bool noDelay /*= false*/)
{
    if (me->HasUnitState(UNIT_STATE_CASTING))
        return true;

    if (noDelay || me->isAttackReady())
    {
        if (auto spellInfo = sSpellMgr->GetSpellInfo(spell))
        {
            auto actualTarget = target;
            if (!actualTarget)
                actualTarget = me->getVictim();

            if (me->IsWithinCombatRange(actualTarget, spellInfo->GetMaxRange(false, me)))
            {
                me->CastSpell(actualTarget, spell, triggerFlags);
                me->resetAttackTimer();
            }
            else
                return false;
        }
        else
            return false;
    }
    return true;
}

Unit* UnitAI::SelectTarget(SelectAggroTarget targetType, uint32 position, float dist, bool playerOnly, int32 aura)
{
    if (!this)
        return nullptr;
    return SelectTarget(targetType, position, DefaultTargetSelector(me, dist, playerOnly, aura));
}

void UnitAI::SelectTargetList(std::list<Unit*>& targetList, uint32 num, SelectAggroTarget targetType, float dist, bool playerOnly, int32 aura)
{
    SelectTargetList(targetList, DefaultTargetSelector(me, dist, playerOnly, aura), num, targetType);
}

float UnitAI::DoGetSpellMaxRange(uint32 spellId, bool positive)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    return spellInfo ? spellInfo->GetMaxRange(positive, me) : 0;
}

void UnitAI::DoAddAuraToAllHostilePlayers(uint32 spellid)
{
    if (!me->isInCombat())
        return;

    auto& threatlist = me->getThreatManager().getThreatList();
    for (auto & itr : threatlist)
        if (auto unit = Unit::GetUnit(*me, itr->getUnitGuid()))
            if (unit->IsPlayer())
                me->AddAura(spellid, unit);
}

void UnitAI::DoCastToAllHostilePlayers(uint32 spellid, bool triggered)
{
    if (!me->isInCombat())
        return;

    auto& threatlist = me->getThreatManager().getThreatList();
    for (auto & itr : threatlist)
        if (auto unit = Unit::GetUnit(*me, itr->getUnitGuid()))
            if (unit->IsPlayer())
                me->CastSpell(unit, spellid, triggered);
}

void UnitAI::DoCast(Unit* victim, uint32 spellId, bool triggered)
{
    if (!victim || (me->HasUnitState(UNIT_STATE_CASTING) && !triggered))
        return;

    me->CastSpell(victim, spellId, triggered);
}

void UnitAI::DoCastVictim(uint32 spellId, bool triggered)
{
    if (!me->getVictim() || (me->HasUnitState(UNIT_STATE_CASTING) && !triggered))
        return;

    me->CastSpell(me->getVictim(), spellId, triggered);
}

void UnitAI::DoCastAOE(uint32 spellId, bool triggered)
{
    if (!triggered && me->HasUnitState(UNIT_STATE_CASTING))
        return;

    me->CastSpell(static_cast<Unit*>(nullptr), spellId, triggered);
}

void UnitAI::DoFunctionToHostilePlayers(uint8 playersCount, std::function<void(Unit*, Player*)> const& function)
{
    if (!me->isInCombat())
        return;

    auto& threatlist = me->getThreatManager().getThreatList();

    if (playersCount)
        Trinity::Containers::RandomResizeList(threatlist, playersCount);

    for (auto& itr : threatlist)
        if (auto unit = Unit::GetUnit(*me, itr->getUnitGuid()))
            if (auto player = unit->ToPlayer())
                function(me, player);
}

void UnitAI::DoCastTopAggro(uint32 spellId, bool triggered, bool onlyPlayer /*= true*/)
{
    if (Unit* victim = SelectTarget(SELECT_TARGET_TOPAGGRO, 0, 100.0f, onlyPlayer))
        me->CastSpell(victim, spellId, triggered);
}

void UnitAI::DoCast(uint32 spellId)
{
    Unit* target = nullptr;
    //TC_LOG_ERROR(LOG_FILTER_GENERAL, "aggre %u %u", spellId, (uint32)AISpellInfo[spellId].target);
    switch (AISpellInfo[spellId].target)
    {
    default:
    case AITARGET_SELF:
        target = me;
        break;
    case AITARGET_VICTIM:
        target = me->getVictim();
        break;
    case AITARGET_ENEMY:
        if (auto spellInfo = sSpellMgr->GetSpellInfo(spellId))
            target = SelectTarget(SELECT_TARGET_RANDOM, 0, spellInfo->GetMaxRange(false, me), spellInfo->HasAttribute(SPELL_ATTR3_ONLY_TARGET_PLAYERS) != 0);
        break;
    case AITARGET_ALLY:
        target = me;
        break;
    case AITARGET_BUFF:
        target = me;
        break;
    case AITARGET_DEBUFF:
    {
        if (auto spellInfo = sSpellMgr->GetSpellInfo(spellId))
        {
            DefaultTargetSelector targetSelector(me, spellInfo->GetMaxRange(false), spellInfo->HasAttribute(SPELL_ATTR3_ONLY_TARGET_PLAYERS) != 0, -static_cast<int32>(spellId));
            if (!(spellInfo->HasAuraInterruptFlag(SpellAuraInterruptFlags(AURA_INTERRUPT_FLAG_NOT_VICTIM))) && targetSelector(me->getVictim()))
                target = me->getVictim();
            else
                target = SelectTarget(SELECT_TARGET_RANDOM, 0, targetSelector);
        }
        break;
    }
    }

    if (target)
        me->CastSpell(target, spellId, false);
}

#define UPDATE_TARGET(a) {if (AIInfo->target<a) AIInfo->target=a;}

void UnitAI::FillAISpellInfo()
{
    AISpellInfo = new AISpellInfoType[sSpellMgr->GetSpellInfoStoreSize()];

    AISpellInfoType* AIInfo = AISpellInfo;

    for (uint32 i = 0; i < sSpellMgr->GetSpellInfoStoreSize(); ++i, ++AIInfo)
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(i);
        if (!spellInfo)
            continue;

        if (spellInfo->HasAttribute(SPELL_ATTR0_CANT_USED_IN_COMBAT))
            AIInfo->condition = AICOND_EVADE;
        else if (spellInfo->HasAttribute(SPELL_ATTR0_CASTABLE_WHILE_DEAD))
            AIInfo->condition = AICOND_DIE;
        else if (spellInfo->IsPassive() || spellInfo->GetDuration() == -1)
            AIInfo->condition = AICOND_AGGRO;
        else
            AIInfo->condition = AICOND_COMBAT;

        if (AIInfo->cooldown < spellInfo->Cooldowns.RecoveryTime)
            AIInfo->cooldown = spellInfo->Cooldowns.RecoveryTime;

        int32 casttime = spellInfo->CalcCastTime();
        if (!casttime)
        {
            if (spellInfo->IsControlSpell())
            {
                if (AIInfo->cooldown < 15000)
                    AIInfo->cooldown = 15000;
            }
            else if (spellInfo->IsDamageSpell())
            {
                if (AIInfo->cooldown < 10000)
                    AIInfo->cooldown = 10000;
            }
            else
            {
                if (AIInfo->cooldown < 5000)
                    AIInfo->cooldown = 5000;
            }
        }

        if (spellInfo->IsTargetingArea())
            AIInfo->cooldown = 15000;

        if (casttime < 2000 && AIInfo->cooldown < 2000)
            AIInfo->cooldown = 2000;

        if (!spellInfo->GetMaxRange(false))
            UPDATE_TARGET(AITARGET_SELF)
        else
        {
            bool checkAura = true;
            for (uint32 j = 0; j < MAX_SPELL_EFFECTS; ++j)
            {
                if (spellInfo->EffectMask < uint32(1 << j)) // Prevent circle if effect not exist
                    break;

                if ((spellInfo->EffectMask & (1 << j)) == 0)
                    continue;

                SpellTargetCheckTypes targetType = spellInfo->Effects[j]->TargetA.GetCheckType();
                SpellTargetReferenceTypes targetReference = spellInfo->Effects[j]->TargetA.GetReferenceType();

                if (spellInfo->Effects[j]->Effect != SPELL_EFFECT_APPLY_AURA)
                    checkAura = false;

                if (targetType == TARGET_CHECK_DEFAULT)
                {
                    if (targetReference == TARGET_REFERENCE_TYPE_DEST)
                        UPDATE_TARGET(AITARGET_VICTIM)
                    else
                        UPDATE_TARGET(AITARGET_SELF)
                }
                else if (targetType == TARGET_CHECK_ENEMY)
                    UPDATE_TARGET(AITARGET_VICTIM)
                else if (targetType == TARGET_CHECK_ALLY)
                    UPDATE_TARGET(AITARGET_ALLY)

                    if (spellInfo->Effects[j]->Effect == SPELL_EFFECT_APPLY_AURA && checkAura)
                    {
                        if (targetType == TARGET_CHECK_ENEMY)
                            UPDATE_TARGET(AITARGET_DEBUFF)
                        else if (spellInfo->IsPositive())
                            UPDATE_TARGET(AITARGET_BUFF)
                    }
            }
        }

        for (uint32 j = 0; j < MAX_SPELL_EFFECTS; ++j)
        {
            if (spellInfo->EffectMask < uint32(1 << j)) // Prevent circle if effect not exist
                break;

            if ((spellInfo->EffectMask & (1 << j)) == 0)
                continue;

            switch (spellInfo->Effects[j]->ApplyAuraName)
            {
            case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
                if (SpellInfo const* triggerInfo = sSpellMgr->GetSpellInfo(spellInfo->Effects[j]->TriggerSpell))
                    if (triggerInfo->IsTargetingArea())
                        AIInfo->cooldown = 15000;
                break;
            }
        }
        AIInfo->realCooldown = spellInfo->Cooldowns.RecoveryTime + spellInfo->Cooldowns.StartRecoveryTime;
        AIInfo->maxRange = spellInfo->GetMaxRange(false) * 3 / 4;
    }
}

void UnitAI::AddDelayedEvent(Minutes delayTime, std::function<void()>&& function)
{
    AddDelayedEvent(std::chrono::duration_cast<Seconds>(delayTime), std::move(function));
}

void UnitAI::AddDelayedEvent(Seconds delayTime, std::function<void()>&& function)
{
    me->AddDelayedEvent(delayTime.count(), std::move(function));
}

void UnitAI::KillAllDelayedEvents()
{
    me->KillAllDelayedEvents();
}

//Enable PlayerAI when charmed
void PlayerAI::OnCharmed(bool apply) { me->IsAIEnabled = apply; }

bool PlayerAI::UpdateVictim()
{
    if (!owner)
        return false;

    if (!owner->isInCombat())
        return false;

    if (Unit* victim = owner->SelectVictim())
        AttackStart(victim);

    return owner->getVictim();
}

void SimpleCharmedAI::UpdateAI(uint32 /*diff*/)
{
    Creature* charmer = me->GetCharmer()->ToCreature();

    //kill self if charm aura has infinite duration
    if (charmer->IsInEvadeMode())
    {
        if (Unit::AuraEffectList const* auras = me->GetAuraEffectsByType(SPELL_AURA_MOD_CHARM))
        {
            for (Unit::AuraEffectList::const_iterator iter = auras->begin(); iter != auras->end(); ++iter)
                if ((*iter)->GetCasterGUID() == charmer->GetGUID() && (*iter)->GetBase()->IsPermanent())
                {
                    charmer->Kill(me);
                    return;
                }
        }
    }

    if (!charmer->isInCombat())
        me->GetMotionMaster()->MoveFollow(charmer, me->GetFollowDistance(), me->GetFollowAngle());

    Unit* target = me->getVictim();
    if (!target || !charmer->IsValidAttackTarget(target))
        AttackStart(charmer->SelectNearestTargetInAttackDistance());
}

DefaultTargetSelector::DefaultTargetSelector(Unit const* unit, float dist, bool playerOnly, int32 aura) : me(unit), m_dist(dist), m_playerOnly(playerOnly), m_aura(aura)
{
}

bool DefaultTargetSelector::operator()(Unit const* target) const
{
    if (!me)
        return false;
    if (!target)
        return false;
    if (m_playerOnly && (!target->IsPlayer()))
        return false;
    if (m_dist > 0.0f && !me->IsWithinCombatRange(target, m_dist))
        return false;
    if (m_dist < 0.0f && me->IsWithinCombatRange(target, -m_dist))
        return false;
    if (m_aura)
    {
        if (m_aura > 0)
        {
            if (!target->HasAura(m_aura))
                return false;
        }
        else
        {
            if (target->HasAura(-m_aura))
                return false;
        }
    }
    return true;
}

SpellTargetSelector::SpellTargetSelector(Unit* caster, uint32 spellId) :
    _caster(caster), _spellInfo(sSpellMgr->GetSpellInfo(spellId))
{
    ASSERT(_spellInfo);
}

bool SpellTargetSelector::operator()(Unit const* target) const
{
    if (!target)
        return false;

    if (_spellInfo->CheckTarget(_caster, target) != SPELL_CAST_OK)
        return false;

    // copypasta from Spell::CheckRange
    uint32 range_type = _spellInfo->Misc.RangeEntry ? _spellInfo->Misc.RangeEntry->Flags : 0;
    float max_range = _caster->GetSpellMaxRangeForTarget(target, _spellInfo);
    float min_range = _caster->GetSpellMinRangeForTarget(target, _spellInfo);

    if (target && target != _caster)
    {
        if (range_type == SPELL_RANGE_MELEE)
        {
            // Because of lag, we can not check too strictly here.
            if (!_caster->IsWithinMeleeRange(target, max_range))
                return false;
        }
        else if (!_caster->IsWithinCombatRange(target, max_range))
            return false;

        if (range_type == SPELL_RANGE_RANGED)
        {
            if (_caster->IsWithinMeleeRange(target))
                return false;
        }
        else if (min_range && _caster->IsWithinCombatRange(target, min_range)) // skip this check if min_range = 0
            return false;
    }

    return true;
}

bool NonTankTargetSelector::operator()(Unit const* target) const
{
    if (!target)
        return false;

    if (_playerOnly && !target->IsPlayer())
        return false;

    return target != _source->getVictim();
}

bool TankTargetSelector::operator()(Unit const* target) const
{
    if (!target)
        return false;

    if (_playerOnly && !target->IsPlayer())
        return false;

    return target == _source->getVictim();
}
