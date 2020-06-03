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

#ifndef BATTLEFIELD_MGR_H_
#define BATTLEFIELD_MGR_H_

#include "Battlefield.h"

class Player;
class GameObject;
class Creature;
class ZoneScript;
struct GossipMenuItems;

class BattlefieldMgr
{
  private:
    BattlefieldMgr();
  public:
    static BattlefieldMgr* instance()
    {
        static BattlefieldMgr instance;
        return &instance;
    }

    ~BattlefieldMgr();

    void InitBattlefield();
    void HandlePlayerEnterZone(ObjectGuid guid, uint32 areaflag);
    void HandlePlayerLeaveZone(ObjectGuid guid, uint32 areaflag);
    void EventPlayerLoggedOut(Player* player);
    Battlefield* GetBattlefieldToZoneId(uint32 zoneid);
    Battlefield* GetBattlefieldByBattleId(uint32 battleid);
    Battlefield* GetBattlefieldByQueueID(uint64 const& id);

    ZoneScript* GetZoneScript(uint32 zoneId);

    void AddZone(uint32 zoneid, Battlefield* handle);

    std::set<Battlefield*>* GetBattlefieldMap(uint32 MapID);

    void Update(uint32 diff);

    typedef std::vector<Battlefield*>BattlefieldSet;
    typedef std::map<uint32 /*zoneid*/ , Battlefield*>BattlefieldZone;
    typedef std::map<uint32 /* mapId */, std::set<Battlefield*>> BattlefieldMap;
  private:
    BattlefieldSet m_BattlefieldSet;
    BattlefieldZone m_BattlefieldZone;
    BattlefieldMap m_BattlefieldMap;
    uint32 m_UpdateTimer;
};

#define sBattlefieldMgr BattlefieldMgr::instance()

#endif
