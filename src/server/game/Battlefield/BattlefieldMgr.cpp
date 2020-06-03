/*
 * Copyright (C) 2008-2010 TrinityCore <http://www.trinitycore.org/>
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

#include "BattlefieldMgr.h"
#include "Zones/BattlefieldWG.h"
#include "Zones/BattlefieldTB.h"
#include "ObjectMgr.h"
#include "Player.h"

BattlefieldMgr::BattlefieldMgr()
{
    m_UpdateTimer = 0;
}

BattlefieldMgr::~BattlefieldMgr()
{
    for (auto const& v : m_BattlefieldSet)
        delete v;
}

void BattlefieldMgr::InitBattlefield()
{
    Battlefield* pBf = new BattlefieldWG;
    if (!pBf->SetupBattlefield())
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Battlefield : Wintergrasp init failed.");
        delete pBf;
    }
    else
    {
        m_BattlefieldSet.push_back(pBf);
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Battlefield : Wintergrasp successfully initiated.");
    }

    pBf = new BattlefieldTB;
    if (!pBf->SetupBattlefield())
    {
        TC_LOG_DEBUG(LOG_FILTER_BATTLEFIELD, "Battlefield : Tol Barad init failed.");
        delete pBf;
    }
    else
    {
        m_BattlefieldSet.push_back(pBf);
        TC_LOG_DEBUG(LOG_FILTER_BATTLEFIELD, "Battlefield : Tol Barad successfully initiated.");
    } 
}

void BattlefieldMgr::AddZone(uint32 zoneid, Battlefield *handle)
{
    m_BattlefieldZone[zoneid] = handle;
    if (AreaTableEntry const* areaEntry = sAreaTableStore.LookupEntry(zoneid))
    {
        m_BattlefieldMap[areaEntry->ContinentID].insert(handle);
        if (Map* map = sMapMgr->FindMap(areaEntry->ContinentID, 0))
        {
            map->BattlefieldList = GetBattlefieldMap(areaEntry->ContinentID);
            handle->SetMap(map);
        }
    }
}

std::set<Battlefield*>* BattlefieldMgr::GetBattlefieldMap(uint32 MapID)
{
    BattlefieldMap::iterator itr = m_BattlefieldMap.find(MapID);
    if (itr == m_BattlefieldMap.end())
        return nullptr;

    return &itr->second;
}

void BattlefieldMgr::HandlePlayerEnterZone(ObjectGuid guid, uint32 zoneid)
{
    BattlefieldZone::iterator itr = m_BattlefieldZone.find(zoneid);
    if (itr == m_BattlefieldZone.end())
        return;

    if (!itr->second->IsEnabled())
        return;

    itr->second->HandlePlayerEnterZone(guid, zoneid);
}

void BattlefieldMgr::HandlePlayerLeaveZone(ObjectGuid guid, uint32 zoneid)
{
    BattlefieldZone::iterator itr = m_BattlefieldZone.find(zoneid);
    if (itr == m_BattlefieldZone.end())
        return;

    itr->second->HandlePlayerLeaveZone(guid, zoneid);
}

Battlefield *BattlefieldMgr::GetBattlefieldToZoneId(uint32 zoneid)
{
    BattlefieldZone::iterator itr = m_BattlefieldZone.find(zoneid);
    if (itr == m_BattlefieldZone.end())
        return nullptr;

    if (!itr->second->IsEnabled())
        return nullptr;

    return itr->second;
}

Battlefield *BattlefieldMgr::GetBattlefieldByBattleId(uint32 battleid)
{
    for (BattlefieldSet::iterator itr = m_BattlefieldSet.begin(); itr != m_BattlefieldSet.end(); ++itr)
        if ((*itr)->GetBattleId() == battleid)
            return (*itr);

    return nullptr;
}

Battlefield* BattlefieldMgr::GetBattlefieldByQueueID(uint64 const& queueID)
{
    for (Battlefield* bf : m_BattlefieldSet)
        if (bf->GetQueueID() == queueID)
            return bf;

    return nullptr;
}

void BattlefieldMgr::Update(uint32 diff)
{
    m_UpdateTimer += diff;
    if (m_UpdateTimer > BATTLEFIELD_OBJECTIVE_UPDATE_INTERVAL)
    {
        for (BattlefieldSet::iterator itr = m_BattlefieldSet.begin(); itr != m_BattlefieldSet.end(); ++itr)
            if ((*itr)->IsEnabled())
                (*itr)->Update(m_UpdateTimer);
        m_UpdateTimer = 0;
    }
}

ZoneScript *BattlefieldMgr::GetZoneScript(uint32 zoneId)
{
    if (m_BattlefieldZone.empty())
        return nullptr;

    BattlefieldZone::iterator itr = m_BattlefieldZone.find(zoneId);
    if (itr != m_BattlefieldZone.end())
        return itr->second;
    return nullptr;
}

void BattlefieldMgr::EventPlayerLoggedOut(Player * player)
{
    if (m_BattlefieldSet.empty())
        return;

    for (BattlefieldSet::iterator itr = m_BattlefieldSet.begin(); itr != m_BattlefieldSet.end(); ++itr)
        if ((*itr)->IsEnabled())
            (*itr)->AskToLeaveQueue(player);
}