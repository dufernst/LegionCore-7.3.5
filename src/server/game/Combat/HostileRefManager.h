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

#ifndef _HOSTILEREFMANAGER
#define _HOSTILEREFMANAGER

#include "RefManager.h"

class Unit;
class ThreatManager;
class HostileReference;
class SpellInfo;

class HostileRefManager : public RefManager<Unit, ThreatManager>
{
    Unit* iOwner;
    std::recursive_mutex i_threat_lock;
public:
    explicit HostileRefManager(Unit* owner);
    ~HostileRefManager();

    Unit* getOwner();

    void threatAssist(Unit* victim, float baseThreat, SpellInfo const* threatSpell = nullptr);

    void addTempThreat(float threat, bool apply);

    void addThreatPercent(int32 percent);

    void deleteReferences();

    void deleteReferencesForFaction(uint32 faction);

    HostileReference* getFirst();

    void updateThreatTables();

    void setOnlineOfflineState(bool isOnline);

    void setOnlineOfflineState(Unit* creature, bool isOnline);

    bool HasTarget(Unit* creature);

    void deleteReference(Unit* creature);

    void UpdateVisibility();
};

#endif
