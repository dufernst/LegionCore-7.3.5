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

#include "UnitEvents.h"

UnitBaseEvent::UnitBaseEvent(uint32 pType)
{
    iType = pType;
}

uint32 UnitBaseEvent::getType() const
{
    return iType;
}

bool UnitBaseEvent::matchesTypeMask(uint32 pMask) const
{
    return (iType & pMask) != 0;
}

void UnitBaseEvent::setType(uint32 pType)
{
    iType = pType;
}

ThreatRefStatusChangeEvent::ThreatRefStatusChangeEvent(uint32 pType): UnitBaseEvent(pType), iThreatManager(nullptr)
{
    iHostileReference = nullptr;
}

ThreatRefStatusChangeEvent::ThreatRefStatusChangeEvent(uint32 pType, HostileReference* pHostileReference): UnitBaseEvent(pType), iThreatManager(nullptr)
{
    iHostileReference = pHostileReference;
}

ThreatRefStatusChangeEvent::ThreatRefStatusChangeEvent(uint32 pType, HostileReference* pHostileReference, float pValue): UnitBaseEvent(pType), iThreatManager(nullptr)
{
    iHostileReference = pHostileReference;
    iFValue = pValue;
}

ThreatRefStatusChangeEvent::ThreatRefStatusChangeEvent(uint32 pType, HostileReference* pHostileReference, bool pValue): UnitBaseEvent(pType), iThreatManager(nullptr)
{
    iHostileReference = pHostileReference;
    iBValue = pValue;
}

int32 ThreatRefStatusChangeEvent::getIValue() const
{
    return iIValue;
}

float ThreatRefStatusChangeEvent::getFValue() const
{
    return iFValue;
}

bool ThreatRefStatusChangeEvent::getBValue() const
{
    return iBValue;
}

void ThreatRefStatusChangeEvent::setBValue(bool pValue)
{
    iBValue = pValue;
}

HostileReference* ThreatRefStatusChangeEvent::getReference() const
{
    return iHostileReference;
}

void ThreatRefStatusChangeEvent::setThreatManager(ThreatManager* pThreatManager)
{
    iThreatManager = pThreatManager;
}

ThreatManager* ThreatRefStatusChangeEvent::getThreatManager() const
{
    return iThreatManager;
}

ThreatManagerEvent::ThreatManagerEvent(uint32 pType): ThreatRefStatusChangeEvent(pType), iThreatContainer(nullptr)
{
}

ThreatManagerEvent::ThreatManagerEvent(uint32 pType, HostileReference* pHostileReference): ThreatRefStatusChangeEvent(pType, pHostileReference), iThreatContainer(nullptr)
{
}

void ThreatManagerEvent::setThreatContainer(ThreatContainer* pThreatContainer)
{
    iThreatContainer = pThreatContainer;
}

ThreatContainer* ThreatManagerEvent::getThreatContainer() const
{
    return iThreatContainer;
}
