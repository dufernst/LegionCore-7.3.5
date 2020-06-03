/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

#ifndef _LFGQUEUE_H
#define _LFGQUEUE_H

#include "LFG.h"
#include <cds/gc/hp.h>
#include <cds/container/impl/feldman_hashmap.h>
#include "HashFuctor.h"

namespace lfg
{

enum LfgCompatibility
{
    LFG_COMPATIBILITY_PENDING,
    LFG_INCOMPATIBLES_WRONG_GROUP_SIZE,
    LFG_INCOMPATIBLES_TOO_MUCH_PLAYERS,
    LFG_INCOMPATIBLES_MULTIPLE_LFG_GROUPS,
    LFG_INCOMPATIBLES_HAS_IGNORES,
    LFG_INCOMPATIBLES_NO_ROLES,
    LFG_INCOMPATIBLES_NO_DUNGEONS,
    LFG_COMPATIBLES_WITH_LESS_PLAYERS,                     // Values under this = not compatible (do not modify order)
    LFG_COMPATIBLES_BAD_STATES,
    LFG_COMPATIBLES_MATCH                                  // Must be the last one
};

struct LfgCompatibilityData
{
    LfgCompatibilityData(): compatibility(LFG_COMPATIBILITY_PENDING) { }
    LfgCompatibilityData(LfgCompatibility _compatibility): compatibility(_compatibility) { }
    LfgCompatibilityData(LfgCompatibility _compatibility, LfgRolesMap const& _roles):
        compatibility(_compatibility), roles(_roles) { }

    LfgCompatibility compatibility;
    LfgRolesMap roles;
};

/// Stores player or group queue info
struct LfgQueueData
{
    LfgQueueData();
    LfgQueueData(time_t _joinTime, LfgDungeonSet const& _dungeons, const LfgRolesMap &_roles);

    time_t joinTime;                                       ///< Player queue join time (to calculate wait times)
    uint8 tanks;                                           ///< Tanks needed
    uint8 healers;                                         ///< Healers needed
    uint8 dps;                                             ///< Dps needed
    LfgDungeonSet dungeons;                                ///< Selected Player/Group Dungeon/s
    LfgRolesMap roles;                                     ///< Selected Player Role/s
    uint8 type;                                            ///< Queue dungeon type
    uint8 subType;                                         ///< Queue dungeon subtype
    std::string bestCompatible;                            ///< Best compatible combination of people queued

    uint8 tanksNeeded;
    uint8 healerNeeded;
    uint8 dpsNeeded;
    uint8 minTanksNeeded;
    uint8 minHealerNeeded;
    uint8 minDpsNeeded;
};

struct LfgWaitTime
{
    LfgWaitTime(): time(-1), number(0) { }
    int32 time;                                            ///< Wait time
    uint32 number;                                         ///< Number of people used to get that wait time
};

struct LfgShortageData
{
    LfgShortageData() : totalTanks(0), totalHealers(0), totalDps(0), queueId(0), shortageRoles(0), shortageRolesCalculated(false) { }
    LfgShortageData(uint32 tanks, uint32 healers, uint32 dps, uint32 _queueId) : totalTanks(tanks), totalHealers(healers), totalDps(dps), queueId(_queueId), shortageRoles(0), shortageRolesCalculated(false) { }

    uint32 totalTanks, totalHealers, totalDps, queueId;

    uint8 GetShortageRoles();

private:
    uint8 shortageRoles;
    bool shortageRolesCalculated;
};

typedef std::map<uint32, LfgWaitTime> LfgWaitTimesContainer;
typedef cds::container::FeldmanHashMap< cds::gc::HP, std::string, LfgCompatibilityData, stringTraits > LfgCompatibleContainer;
typedef std::map<ObjectGuid, LfgQueueData> LfgQueueDataContainer;

/**
    Stores all data related to queue
*/
class LFGQueue
{
    public:

        // Add/Remove from queue
        void AddToQueue(ObjectGuid guid);
        void RemoveFromQueue(ObjectGuid guid);
        void AddQueueData(ObjectGuid guid, time_t joinTime, LfgDungeonSet const& dungeons, LfgRolesMap const& rolesMap);
        void RemoveQueueData(ObjectGuid guid);

        // Update Timers (when proposal success)
        void UpdateWaitTimeAvg(int32 waitTime, uint32 dungeonId);
        void UpdateWaitTimeTank(int32 waitTime, uint32 dungeonId);
        void UpdateWaitTimeHealer(int32 waitTime, uint32 dungeonId);
        void UpdateWaitTimeDps(int32 waitTime, uint32 dungeonId);

        // Update Queue timers
        void UpdateQueueTimers(time_t currTime);

        LfgQueueData const* GetQueueData(ObjectGuid guid);
        time_t GetJoinTime(ObjectGuid guid);
        uint8 GetQueueType(ObjectGuid guid);
        uint8 GetQueueSubType(ObjectGuid guid);

        // Find new group
        uint8 FindGroups();

        // Just for debugging purposes
        std::string DumpQueueInfo() const;
        std::string DumpCompatibleInfo(bool full = false) const;

        // Shortage reward
        void UpdateShortageData();
        uint8 GetShortageRoles();
        uint8 IsEligibleForCTAReward(uint8 roles);

        uint32 queueId{};

    private:
        void AddToNewQueue(ObjectGuid guid);
        void AddToCurrentQueue(ObjectGuid guid);
        void RemoveFromNewQueue(ObjectGuid guid);
        void RemoveFromCurrentQueue(ObjectGuid guid);

        void SetCompatibles(std::string const& key, LfgCompatibility compatibles);
        LfgCompatibility GetCompatibles(std::string const& key);
        void RemoveFromCompatibles(ObjectGuid guid);

        void SetCompatibilityData(std::string const& key, LfgCompatibilityData const& compatibles);
        LfgCompatibilityData* GetCompatibilityData(std::string const& key);
        void FindBestCompatibleInQueue(LfgQueueDataContainer::iterator itrQueue);
        void UpdateBestCompatibleInQueue(LfgQueueDataContainer::iterator itrQueue, std::string const& key, LfgRolesMap const& roles);

        LfgCompatibility FindNewGroups(GuidList& check, GuidList& all);
        LfgCompatibility CheckCompatibility(GuidList check);

        // Queue
        LfgQueueDataContainer QueueDataStore;              ///< Queued groups
        LfgCompatibleContainer CompatibleMapStore;         ///< Compatible dungeons
        LfgShortageData ShortageData;                      ///< Roles which currently experience shortage

        LfgWaitTimesContainer waitTimesAvgStore;           ///< Average wait time to find a group queuing as multiple roles
        LfgWaitTimesContainer waitTimesTankStore;          ///< Average wait time to find a group queuing as tank
        LfgWaitTimesContainer waitTimesHealerStore;        ///< Average wait time to find a group queuing as healer
        LfgWaitTimesContainer waitTimesDpsStore;           ///< Average wait time to find a group queuing as dps
        GuidList currentQueueStore;                        ///< Ordered list. Used to find groups
        GuidList newToQueueStore;                          ///< New groups to add to queue
        std::recursive_mutex m_lock;
};

} // namespace lfg

#endif
