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

#include "ThreatManager.h"
#include "Unit.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "Map.h"
#include "Player.h"
#include "ObjectAccessor.h"
#include "UnitEvents.h"
#include "SpellMgr.h"

float ThreatCalcHelper::calcThreat(Unit* hatedUnit, Unit* /*hatingUnit*/, float threat, SpellSchoolMask schoolMask, SpellInfo const* threatSpell)
{
    if (threatSpell)
    {
        if (auto threatEntry = sSpellMgr->GetSpellThreatEntry(threatSpell->Id))
            if (threatEntry->pctMod != 1.0f)
                threat *= threatEntry->pctMod;

        if (Player* modOwner = hatedUnit->GetSpellModOwner())
            modOwner->ApplySpellMod(threatSpell->Id, SPELLMOD_THREAT, threat);
    }

    return hatedUnit->ApplyTotalThreatModifier(threat, schoolMask);
}

bool ThreatCalcHelper::isValidProcess(Unit* hatedUnit, Unit* hatingUnit, SpellInfo const* threatSpell)
{
    if (!hatedUnit || !hatingUnit)
        return false;

    if (hatedUnit == hatingUnit)
        return false;

    if (hatedUnit->IsPlayer() && hatedUnit->ToPlayer()->isGameMaster())
        return false;

    if (!hatedUnit->isAlive() || !hatingUnit->isAlive())
        return false;

    if (!hatedUnit->IsInMap(hatingUnit) || !hatedUnit->InSamePhase(hatingUnit))
        return false;

    if (threatSpell && threatSpell->HasAttribute(SPELL_ATTR1_NO_THREAT))
        return false;

    ASSERT(hatingUnit->IsCreature() || hatingUnit->IsPlayer());

    return true;
}

HostileReference::HostileReference(Unit* refUnit, ThreatManager* threatManager, float threat)
{
    iThreat = threat;
    iTempThreatModifier = 0.0f;
    link(refUnit, threatManager);
    iUnitGuid = refUnit->GetGUID();
    iOnline = true;
    iAccessible = true;
}

void HostileReference::targetObjectBuildLink()
{
    getTarget()->addHatedBy(this);
}

void HostileReference::targetObjectDestroyLink()
{
    getTarget()->removeHatedBy(this);
}

void HostileReference::sourceObjectDestroyLink()
{
    setOnlineOfflineState(false);
}

void HostileReference::fireStatusChanged(ThreatRefStatusChangeEvent& threatRefStatusChangeEvent)
{
    if (getSource())
        getSource()->processThreatEvent(&threatRefStatusChangeEvent);
}

void HostileReference::addThreat(float modThreat)
{
    iThreat += modThreat;
    if (!isOnline())
        updateOnlineStatus();
    if (modThreat != 0.0f)
    {
        ThreatRefStatusChangeEvent event(UEV_THREAT_REF_THREAT_CHANGE, this, modThreat);
        fireStatusChanged(event);
    }

    if (isValid() && modThreat >= 0.0f)
    {
        Unit* victimOwner = getTarget()->GetCharmerOrOwner();
        if (victimOwner && victimOwner->isAlive())
            getSource()->addThreat(victimOwner, 0.0f);     // create a threat to the owner of a pet, if the pet attacks
    }
}

void HostileReference::setThreat(float threat)
{
    addThreat(threat - getThreat());
}

void HostileReference::addThreatPercent(int32 percent)
{
    float tmpThreat = iThreat;
    AddPct(tmpThreat, percent);
    addThreat(tmpThreat - iThreat);
}

float HostileReference::getThreat() const
{
    return iThreat;
}

bool HostileReference::isOnline() const
{
    return iOnline;
}

bool HostileReference::isAccessible() const
{
    return iAccessible;
}

void HostileReference::setTempThreat(float threat)
{
    addTempThreat(threat - getThreat());
}

void HostileReference::addTempThreat(float threat)
{
    iTempThreatModifier = threat;
    if (iTempThreatModifier != 0.0f)
        addThreat(iTempThreatModifier);
}

void HostileReference::resetTempThreat()
{
    if (iTempThreatModifier != 0.0f)
    {
        addThreat(-iTempThreatModifier);
        iTempThreatModifier = 0.0f;
    }
}

float HostileReference::getTempThreatModifier()
{
    return iTempThreatModifier;
}

void HostileReference::updateOnlineStatus()
{
    bool online = false;
    bool accessible = false;

    if (!isValid())
        if (Unit* target = ObjectAccessor::GetUnit(*getSourceUnit(), getUnitGuid()))
            link(target, getSource());

    if (isValid() && (!getTarget()->IsPlayer() || !getTarget()->ToPlayer()->isGameMaster()) && !getTarget()->HasUnitState(UNIT_STATE_IN_FLIGHT) && getTarget()->IsInMap(getSourceUnit()) && getTarget()->InSamePhase(getSourceUnit()))
    {
        if (Creature* creature = getSourceUnit()->ToCreature())
        {
            online = getTarget()->isInAccessiblePlaceFor(creature);
            if (!online)
            {
                if (creature->IsWithinCombatRange(getTarget(), creature->m_CombatDistance))
                    online = true;                              // not accessible but stays online
            }
            else
                accessible = true;
        }
    }
    setAccessibleState(accessible);
    setOnlineOfflineState(online);
}

void HostileReference::setOnlineOfflineState(bool isOnline)
{
    if (iOnline != isOnline)
    {
        iOnline = isOnline;
        if (!iOnline)
            setAccessibleState(false);                      // if not online that not accessable as well

        ThreatRefStatusChangeEvent event(UEV_THREAT_REF_ONLINE_STATUS, this);
        fireStatusChanged(event);
    }
}

void HostileReference::setAccessibleState(bool isAccessible)
{
    if (iAccessible != isAccessible)
    {
        iAccessible = isAccessible;

        ThreatRefStatusChangeEvent event(UEV_THREAT_REF_ASSECCIBLE_STATUS, this);
        fireStatusChanged(event);
    }
}

bool HostileReference::operator==(const HostileReference& hostileRef) const
{
    return hostileRef.getUnitGuid() == getUnitGuid();
}

ObjectGuid HostileReference::getUnitGuid() const
{
    return iUnitGuid;
}

void HostileReference::removeReference()
{
    invalidate();

    ThreatRefStatusChangeEvent event(UEV_THREAT_REF_REMOVE_FROM_LIST, this);
    fireStatusChanged(event);
}

HostileReference* HostileReference::next()
{
    return dynamic_cast<HostileReference*>(Reference<Unit, ThreatManager>::next());
}

Unit* HostileReference::getSourceUnit()
{
    return (getSource()->getOwner());
}

void ThreatContainer::remove(HostileReference* hostileRef)
{
    std::lock_guard<std::recursive_mutex> guard(i_threat_lock);
    iThreatList.remove(hostileRef);
}

void ThreatContainer::addReference(HostileReference* hostileRef)
{
    std::lock_guard<std::recursive_mutex> guard(i_threat_lock);
    iThreatList.push_back(hostileRef);
}

void ThreatContainer::clearReferences()
{
    std::lock_guard<std::recursive_mutex> guard(i_threat_lock);
    for (std::list<HostileReference*>::const_iterator i = iThreatList.begin(); i != iThreatList.end(); ++i)
    {
        (*i)->unlink();
        delete (*i);
    }

    iThreatList.clear();
}

HostileReference* ThreatContainer::getReferenceByTarget(Unit* victim)
{
    if (!victim)
        return nullptr;

    std::lock_guard<std::recursive_mutex> guard(i_threat_lock);
    ObjectGuid guid = victim->GetGUID();
    for (std::list<HostileReference*>::const_iterator i = iThreatList.begin(); i != iThreatList.end(); ++i)
        if ((*i) && (*i)->getUnitGuid() == guid)
            return (*i);

    return nullptr;
}

std::list<HostileReference*>& ThreatContainer::getThreatList()
{
    return iThreatList;
}

HostileReference* ThreatContainer::addThreat(Unit* victim, float threat)
{
    auto ref = getReferenceByTarget(victim);
    if (ref)
        ref->addThreat(threat);
    return ref;
}

void ThreatContainer::modifyThreatPercent(Unit* victim, int32 percent)
{
    if (auto ref = getReferenceByTarget(victim))
        ref->addThreatPercent(percent);
}

void ThreatContainer::update()
{
    if (iDirty && iThreatList.size() > 1)
        iThreatList.sort([](HostileReference const* a, HostileReference const* b)
        {
            if (!a)
                return false;
            if (!b)
                return true;
            return a->getThreat() > b->getThreat();
        });

    iDirty = false;
}

ThreatContainer::ThreatContainer()
{
    iDirty = false;
}

ThreatContainer::~ThreatContainer()
{
    clearReferences();
}

HostileReference* ThreatContainer::selectNextVictim(Creature* attacker, HostileReference* currentVictim)
{
    HostileReference* currentRef = nullptr;
    bool found = false;
    bool noPriorityTargetFound = false;

    if (iThreatList.empty())
        return nullptr;

    std::list<HostileReference*>::const_iterator lastRef = iThreatList.end();
    --lastRef;

    for (std::list<HostileReference*>::const_iterator iter = iThreatList.begin(); iter != iThreatList.end() && !found;)
    {
        currentRef = (*iter);

        Unit* target = currentRef->getTarget();
        ASSERT(target);                                     // if the ref has status online the target must be there !

        // some units are prefered in comparison to others
        if (!noPriorityTargetFound && (target->IsImmunedToDamage(attacker->GetMeleeDamageSchoolMask()) || target->isCharmed() || target->HasNegativeAuraWithInterruptFlag(AURA_INTERRUPT_FLAG_TAKE_DAMAGE) || attacker->IsAIEnabled && !attacker->AI()->AllowSelectNextVictim(target)))
        {
            if (iter != lastRef)
            {
                // current victim is a second choice target, so don't compare threat with it below
                if (currentRef == currentVictim)
                    currentVictim = nullptr;
                ++iter;
                continue;
            }
            // if we reached to this point, everyone in the threatlist is a second choice target. In such a situation the target with the highest threat should be attacked.
            noPriorityTargetFound = true;
            iter = iThreatList.begin();
            continue;
        }

        if (attacker->canCreatureAttack(target))           // skip non attackable currently targets
        {
            if (currentVictim)                              // select 1.3/1.1 better target in comparison current target
            {
                // list sorted and and we check current target, then this is best case
                if (currentVictim == currentRef || currentRef->getThreat() <= 1.1f * currentVictim->getThreat())
                {
                    if (currentVictim != currentRef && attacker->canCreatureAttack(currentVictim->getTarget()))
                        currentRef = currentVictim;            // for second case, if currentvictim is attackable

                    found = true;
                    break;
                }

                if (currentRef->getThreat() > 1.3f * currentVictim->getThreat() || (currentRef->getThreat() > 1.1f * currentVictim->getThreat() && attacker->IsWithinMeleeRange(target)))
                {                                           //implement 110% threat rule for targets in melee range
                    found = true;                           //and 130% rule for targets in ranged distances
                    break;                                  //for selecting alive targets
                }
            }
            else                                            // select any
            {
                found = true;
                break;
            }
        }
        ++iter;
    }
    if (!found)
        currentRef = nullptr;

    return currentRef;
}

void ThreatContainer::setDirty(bool isDirty)
{
    iDirty = isDirty;
}

bool ThreatContainer::isDirty() const
{
    return iDirty;
}

bool ThreatContainer::empty() const
{
    return iThreatList.empty();
}

HostileReference* ThreatContainer::getMostHated()
{
    return iThreatList.empty() ? nullptr : iThreatList.front();
}

ThreatManager::ThreatManager(Unit* owner) : iCurrentVictim(nullptr), iOwner(owner), iUpdateTimer(THREAT_UPDATE_INTERVAL)
{
}

ThreatManager::~ThreatManager()
{
    clearReferences();
}

void ThreatManager::clearReferences()
{
    iThreatContainer.clearReferences();
    iThreatOfflineContainer.clearReferences();
    iCurrentVictim = nullptr;
    iUpdateTimer = THREAT_UPDATE_INTERVAL;
}

void ThreatManager::addThreat(Unit* victim, float threat, SpellSchoolMask schoolMask, SpellInfo const* threatSpell)
{
    if (!ThreatCalcHelper::isValidProcess(victim, getOwner(), threatSpell))
        return;

    doAddThreat(victim, ThreatCalcHelper::calcThreat(victim, iOwner, threat, schoolMask, threatSpell));
}

void ThreatManager::doAddThreat(Unit* victim, float threat, SpellSchoolMask schoolMask, SpellInfo const* threatSpell)
{
    uint32 reducedThreadPercent = victim->GetReducedThreatPercent();

    if (threat > 0.0f)
    {
        if (auto creature = iOwner->ToCreature())
        {
            if (victim->IsPlayer() && !iOwner->GetThreatTarget(victim->GetGUID()))
            {
                iOwner->AddThreatTarget(victim->GetGUID());
                if (creature->IsPersonal())
                    iOwner->UpdateMaxHealth();
            }
        }
    }

    if (auto creature = iOwner->ToCreature())
    {
        if (creature->IsAIEnabled)
            creature->AI()->OnAddThreat(victim, threat, schoolMask, threatSpell);
    }

    // must check > 0.0f, otherwise dead loop
    if (threat > 0.0f && reducedThreadPercent)
    {
        Unit* redirectTarget = victim->GetMisdirectionTarget();
        if(redirectTarget)
        {
            if (int32 threatMod = redirectTarget->GetMaxNegativeAuraModifier(SPELL_AURA_MOD_THREAT))
            {
                if (threatMod < -100)
                    threatMod = -100;

                threat *= float(reducedThreadPercent + threatMod) / 100.0f;
            }
        }
        float reducedThreat = threat * reducedThreadPercent / 100.0f;
        threat -= reducedThreat;
        if (redirectTarget)
            _addThreat(redirectTarget, reducedThreat);
    }

    _addThreat(victim, threat);
}

void ThreatManager::_addThreat(Unit* victim, float threat)
{
    auto ref = iThreatContainer.addThreat(victim, threat);
    if (!ref)
        ref = iThreatOfflineContainer.addThreat(victim, threat);

    if (!ref) // there was no ref => create a new one
    {
        // threat has to be 0 here
        auto hostileRef = new HostileReference(victim, this, 0);
        iThreatContainer.addReference(hostileRef);
        hostileRef->addThreat(threat); // now we add the real threat
        if (victim->IsPlayer() && victim->ToPlayer()->isGameMaster())
            hostileRef->setOnlineOfflineState(false); // GM is always offline
    }
}

void ThreatManager::modifyThreatPercent(Unit* victim, int32 percent)
{
    iThreatContainer.modifyThreatPercent(victim, percent);
}

Unit* ThreatManager::getHostilTarget()
{
    std::lock_guard<std::recursive_mutex> guard(iThreatContainer.i_threat_lock);
    iThreatContainer.update();
    auto nextVictim = iThreatContainer.selectNextVictim(getOwner()->ToCreature(), getCurrentVictim());
    setCurrentVictim(nextVictim);
    return getCurrentVictim() != nullptr ? getCurrentVictim()->getTarget() : nullptr;
}

float ThreatManager::getThreat(Unit* victim, bool alsoSearchOfflineList)
{
    float threat = 0.0f;
    auto ref = iThreatContainer.getReferenceByTarget(victim);
    if (!ref && alsoSearchOfflineList)
        ref = iThreatOfflineContainer.getReferenceByTarget(victim);
    if (ref)
        threat = ref->getThreat();
    return threat;
}

bool ThreatManager::isThreatListEmpty()
{
    return iThreatContainer.empty();
}

void ThreatManager::tauntApply(Unit* taunter)
{
    auto ref = iThreatContainer.getReferenceByTarget(taunter);
    if (getCurrentVictim() && ref && (ref->getThreat() < getCurrentVictim()->getThreat()))
        if (ref->getTempThreatModifier() == 0.0f) // Ok, temp threat is unused
            ref->setTempThreat(getCurrentVictim()->getThreat());
}

void ThreatManager::tauntFadeOut(Unit* taunter)
{
    auto ref = iThreatContainer.getReferenceByTarget(taunter);
    if (ref)
        ref->resetTempThreat();
}

void ThreatManager::setCurrentVictim(HostileReference* pHostileReference)
{
    if (pHostileReference && pHostileReference != iCurrentVictim)
        iOwner->SendHighestThreatUpdate(pHostileReference);
    iCurrentVictim = pHostileReference;
}

void ThreatManager::setDirty(bool isDirty)
{
    iThreatContainer.setDirty(isDirty);
}

void ThreatManager::processThreatEvent(ThreatRefStatusChangeEvent* threatRefStatusChangeEvent)
{
    threatRefStatusChangeEvent->setThreatManager(this);     // now we can set the threat manager

    auto hostilRef = threatRefStatusChangeEvent->getReference();

    switch (threatRefStatusChangeEvent->getType())
    {
        case UEV_THREAT_REF_THREAT_CHANGE:
            if ((getCurrentVictim() == hostilRef && threatRefStatusChangeEvent->getFValue() < 0.0f) || (getCurrentVictim() != hostilRef && threatRefStatusChangeEvent->getFValue() > 0.0f))
                setDirty(true);                             // the order in the threat list might have changed
            break;
        case UEV_THREAT_REF_ONLINE_STATUS:
            if (!hostilRef->isOnline())
            {
                if (hostilRef == getCurrentVictim())
                {
                    setCurrentVictim(nullptr);
                    setDirty(true);
                }
                iOwner->SendThreatRemove(hostilRef);
                iThreatContainer.remove(hostilRef);
                iThreatOfflineContainer.addReference(hostilRef);
            }
            else
            {
                if (getCurrentVictim() && hostilRef->getThreat() > (1.1f * getCurrentVictim()->getThreat()))
                    setDirty(true);
                iThreatContainer.addReference(hostilRef);
                iThreatOfflineContainer.remove(hostilRef);
            }
            break;
        case UEV_THREAT_REF_REMOVE_FROM_LIST:
            if (hostilRef == getCurrentVictim())
            {
                setCurrentVictim(nullptr);
                setDirty(true);
            }
            iOwner->SendThreatRemove(hostilRef);
            if (hostilRef->isOnline())
                iThreatContainer.remove(hostilRef);
            else
                iThreatOfflineContainer.remove(hostilRef);
            break;
        default:
            break;
    }
}

bool ThreatManager::isNeedUpdateToClient(uint32 time)
{
    if (isThreatListEmpty())
        return false;

    if (time >= iUpdateTimer)
    {
        iUpdateTimer = THREAT_UPDATE_INTERVAL;
        return true;
    }
    iUpdateTimer -= time;
    return false;
}

HostileReference* ThreatManager::getCurrentVictim()
{
    return iCurrentVictim;
}

Unit* ThreatManager::getOwner()
{
    return iOwner;
}

void ThreatManager::resetAllAggro()
{
    auto& threatList = getThreatList();
    if (threatList.empty())
        return;

    for (auto& itr : threatList)
        itr->setThreat(0);

    setDirty(true);
}

std::list<HostileReference*>& ThreatManager::getThreatList()
{
    return iThreatContainer.getThreatList();
}

std::list<HostileReference*>& ThreatManager::getOfflineThreatList()
{
    return iThreatOfflineContainer.getThreatList();
}

ThreatContainer& ThreatManager::getOnlineContainer()
{
    return iThreatContainer;
}

ThreatContainer& ThreatManager::getOfflineContainer()
{
    return iThreatOfflineContainer;
}
