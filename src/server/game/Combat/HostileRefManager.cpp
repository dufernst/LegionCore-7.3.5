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

#include "HostileRefManager.h"
#include "ThreatManager.h"
#include "Unit.h"
#include "SpellInfo.h"

HostileRefManager::HostileRefManager(Unit* owner)
{
    iOwner = owner;
}

HostileRefManager::~HostileRefManager()
{
    deleteReferences();
}

Unit* HostileRefManager::getOwner()
{
    return iOwner;
}

void HostileRefManager::threatAssist(Unit* victim, float baseThreat, SpellInfo const* threatSpell)
{
    if (getSize() == 0 || baseThreat == 0.0f)
        return;

    SpellSchoolMask schoolMask = threatSpell ? threatSpell->GetSchoolMask() : SPELL_SCHOOL_MASK_NORMAL;
    float threat = ThreatCalcHelper::calcThreat(victim, iOwner, baseThreat, schoolMask, threatSpell);

    std::lock_guard<std::recursive_mutex> guard(i_threat_lock);
    HostileReference* ref = getFirst();
    threat /= getSize();
    while (ref)
    {
        if (ref->getSource())
        {
            if (Unit* owner = ref->getSource()->getOwner())
            {
                volatile uint32 entryorguid = owner->IsPlayer() ? owner->GetGUIDLow() : owner->GetEntry();
                if (owner->isAlive() && !owner->IsDelete() && owner->IsInWorld())
                    if (ThreatCalcHelper::isValidProcess(victim, owner, threatSpell))
                        ref->getSource()->doAddThreat(victim, threat, schoolMask, threatSpell);
            }
        }

        ref = ref->next();
    }
}

void HostileRefManager::addTempThreat(float threat, bool apply)
{
    std::lock_guard<std::recursive_mutex> guard(i_threat_lock);
    HostileReference* ref = getFirst();

    while (ref)
    {
        if (apply)
        {
            if (ref->getTempThreatModifier() == 0.0f)
                ref->addTempThreat(threat);
        }
        else
            ref->resetTempThreat();

        ref = ref->next();
    }
}

void HostileRefManager::addThreatPercent(int32 percent)
{
    std::lock_guard<std::recursive_mutex> guard(i_threat_lock);
    HostileReference* ref = getFirst();
    while (ref)
    {
        ref->addThreatPercent(percent);
        ref = ref->next();
    }
}

void HostileRefManager::setOnlineOfflineState(bool isOnline)
{
    std::lock_guard<std::recursive_mutex> guard(i_threat_lock);
    HostileReference* ref = getFirst();
    while (ref)
    {
        ref->setOnlineOfflineState(isOnline);
        ref = ref->next();
    }
}

void HostileRefManager::updateThreatTables()
{
    std::lock_guard<std::recursive_mutex> guard(i_threat_lock);
    HostileReference* ref = getFirst();
    while (ref)
    {
        ref->updateOnlineStatus();
        ref = ref->next();
    }
}

void HostileRefManager::deleteReferences()
{
    std::lock_guard<std::recursive_mutex> guard(i_threat_lock);
    HostileReference* ref = getFirst();
    while (ref)
    {
        HostileReference* nextRef = ref->next();
        ref->removeReference();
        delete ref;
        ref = nextRef;
    }
}

void HostileRefManager::deleteReferencesForFaction(uint32 faction)
{
    std::lock_guard<std::recursive_mutex> guard(i_threat_lock);
    HostileReference* ref = getFirst();
    while (ref)
    {
        HostileReference* nextRef = ref->next();
        if (ref->getSource()->getOwner()->getFactionTemplateEntry()->Faction == faction)
        {
            ref->removeReference();
            delete ref;
        }
        ref = nextRef;
    }
}

HostileReference* HostileRefManager::getFirst()
{
    return reinterpret_cast<HostileReference*>(RefManager<Unit, ThreatManager>::getFirst());
}

void HostileRefManager::deleteReference(Unit* creature)
{
    std::lock_guard<std::recursive_mutex> guard(i_threat_lock);
    HostileReference* ref = getFirst();
    while (ref)
    {
        HostileReference* nextRef = ref->next();
        if (ref->getSource()->getOwner() == creature)
        {
            ref->removeReference();
            delete ref;
            break;
        }
        ref = nextRef;
    }
}

void HostileRefManager::setOnlineOfflineState(Unit* creature, bool isOnline)
{
    std::lock_guard<std::recursive_mutex> guard(i_threat_lock);
    HostileReference* ref = getFirst();
    while (ref)
    {
        HostileReference* nextRef = ref->next();
        if (ref->getSource()->getOwner() == creature)
        {
            ref->setOnlineOfflineState(isOnline);
            break;
        }
        ref = nextRef;
    }
}

void HostileRefManager::UpdateVisibility()
{
    std::lock_guard<std::recursive_mutex> guard(i_threat_lock);
    HostileReference* ref = getFirst();
    while (ref)
    {
        HostileReference* nextRef = ref->next();
        if (!ref->getSource()->getOwner()->canSeeOrDetect(getOwner()))
        {
            ref->setOnlineOfflineState(false);
            nextRef = ref->next();
            //ref->removeReference();
            //delete ref;
        }
        else
            ref->setOnlineOfflineState(true);
        ref = nextRef;
    }
}

bool HostileRefManager::HasTarget(Unit* creature)
{
    HostileReference* ref = getFirst();
    while (ref)
    {
        HostileReference* nextRef = ref->next();
        if (ref->getSource()->getOwner() == creature)
        {
            return true;
        }
        ref = nextRef;
    }
    return false;
}
