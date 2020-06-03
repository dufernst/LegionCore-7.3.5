/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Containers.h"
#include "Group.h"
#include "LFGQueue.h"
#include "LFGMgr.h"
#include "Log.h"
#include "World.h"

namespace lfg
{

/**
   Given a list of guids returns the concatenation using | as delimiter

   @param[in]     check list of guids
   @returns Concatenated string
*/
std::string ConcatenateGuids(GuidList const& check)
{
    if (check.empty())
        return "";

    // need the guids in order to avoid duplicates
    GuidSet guids(check.begin(), check.end());

    std::ostringstream o;

    GuidSet::const_iterator it = guids.begin();
    o << *it;
    for (++it; it != guids.end(); ++it)
        o << '|' << *it;

    return o.str();
}

char const* GetCompatibleString(LfgCompatibility compatibles)
{
    switch (compatibles)
    {
        case LFG_COMPATIBILITY_PENDING:
            return "Pending";
        case LFG_COMPATIBLES_BAD_STATES:
            return "Compatibles (Bad States)";
        case LFG_COMPATIBLES_MATCH:
            return "Match";
        case LFG_COMPATIBLES_WITH_LESS_PLAYERS:
            return "Compatibles (Not enough players)";
        case LFG_INCOMPATIBLES_HAS_IGNORES:
            return "Has ignores";
        case LFG_INCOMPATIBLES_MULTIPLE_LFG_GROUPS:
            return "Multiple Lfg Groups";
        case LFG_INCOMPATIBLES_NO_DUNGEONS:
            return "Incompatible dungeons";
        case LFG_INCOMPATIBLES_NO_ROLES:
            return "Incompatible roles";
        case LFG_INCOMPATIBLES_TOO_MUCH_PLAYERS:
            return "Too much players";
        case LFG_INCOMPATIBLES_WRONG_GROUP_SIZE:
            return "Wrong group size";
        default:
            return "Unknown";
    }
}

void LFGQueue::AddToQueue(ObjectGuid guid)
{
    LfgQueueDataContainer::iterator itQueue = QueueDataStore.find(guid);
    if (itQueue == QueueDataStore.end())
    {
        TC_LOG_ERROR(LOG_FILTER_LFG, "LFGQueue::AddToQueue: Queue data not found for %s", guid.ToString().c_str());
        return;
    }

    AddToNewQueue(guid);
}

void LFGQueue::RemoveFromQueue(ObjectGuid guid)
{
    RemoveFromNewQueue(guid);
    RemoveFromCurrentQueue(guid);
    RemoveFromCompatibles(guid);

    std::ostringstream o;
    o << guid;
    std::string sguid = o.str();

    LfgQueueDataContainer::iterator itDelete = QueueDataStore.end();
    for (LfgQueueDataContainer::iterator itr = QueueDataStore.begin(); itr != QueueDataStore.end(); ++itr)
        if (itr->first != guid)
        {
            if (std::string::npos != itr->second.bestCompatible.find(sguid))
            {
                itr->second.bestCompatible.clear();
                FindBestCompatibleInQueue(itr);
            }
        }
        else
            itDelete = itr;

    if (itDelete != QueueDataStore.end())
        QueueDataStore.erase(itDelete);
}

void LFGQueue::AddToNewQueue(ObjectGuid guid)
{
    newToQueueStore.push_back(guid);
}

void LFGQueue::RemoveFromNewQueue(ObjectGuid guid)
{
    newToQueueStore.remove(guid);
}

void LFGQueue::AddToCurrentQueue(ObjectGuid guid)
{
    currentQueueStore.push_back(guid);
}

void LFGQueue::RemoveFromCurrentQueue(ObjectGuid guid)
{
    currentQueueStore.remove(guid);
}

void LFGQueue::AddQueueData(ObjectGuid guid, time_t joinTime, LfgDungeonSet const& dungeons, LfgRolesMap const& rolesMap)
{
    QueueDataStore[guid] = LfgQueueData(joinTime, dungeons, rolesMap);
    AddToQueue(guid);
}

void LFGQueue::RemoveQueueData(ObjectGuid guid)
{
    LfgQueueDataContainer::iterator it = QueueDataStore.find(guid);
    if (it != QueueDataStore.end())
        QueueDataStore.erase(it);
}

void LFGQueue::UpdateWaitTimeAvg(int32 waitTime, uint32 dungeonId)
{
    LfgWaitTime &wt = waitTimesAvgStore[dungeonId];
    uint32 old_number = wt.number++;
    wt.time = int32((wt.time * old_number + waitTime) / wt.number);
}

void LFGQueue::UpdateWaitTimeTank(int32 waitTime, uint32 dungeonId)
{
    LfgWaitTime &wt = waitTimesTankStore[dungeonId];
    uint32 old_number = wt.number++;
    wt.time = int32((wt.time * old_number + waitTime) / wt.number);
}

void LFGQueue::UpdateWaitTimeHealer(int32 waitTime, uint32 dungeonId)
{
    LfgWaitTime &wt = waitTimesHealerStore[dungeonId];
    uint32 old_number = wt.number++;
    wt.time = int32((wt.time * old_number + waitTime) / wt.number);
}

void LFGQueue::UpdateWaitTimeDps(int32 waitTime, uint32 dungeonId)
{
    LfgWaitTime &wt = waitTimesDpsStore[dungeonId];
    uint32 old_number = wt.number++;
    wt.time = int32((wt.time * old_number + waitTime) / wt.number);
}

/**
   Remove from cached compatible dungeons any entry that contains the given guid

   @param[in]     guid Guid to remove from compatible cache
*/
void LFGQueue::RemoveFromCompatibles(ObjectGuid guid)
{
    std::stringstream out;
    out << guid;
    std::string strGuid = out.str();

    std::lock_guard<std::recursive_mutex> _lock(m_lock);

    TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGQueue::RemoveFromCompatibles: Removing %s", guid.ToString().c_str());
    for (LfgCompatibleContainer::iterator iter = CompatibleMapStore.begin(); iter != CompatibleMapStore.end(); ++iter)
    {
        std::string key = iter->first;
        if (std::string::npos != key.find(strGuid))
            CompatibleMapStore.erase(key);
    }
}

/**
   Stores the compatibility of a list of guids

   @param[in]     key String concatenation of guids (| used as separator)
   @param[in]     compatibles type of compatibility
*/
void LFGQueue::SetCompatibles(std::string const& key, LfgCompatibility compatibles)
{
    std::lock_guard<std::recursive_mutex> _lock(m_lock);
    CompatibleMapStore.update(key, [compatibles](LfgCompatibleContainer::value_type& item, LfgCompatibleContainer::value_type* old)
    {
        if (old)
            item.second.roles = old->second.roles;
        item.second.compatibility = compatibles;
    });
}

void LFGQueue::SetCompatibilityData(std::string const& key, LfgCompatibilityData const& data)
{
    std::lock_guard<std::recursive_mutex> _lock(m_lock);
    CompatibleMapStore.insert_with(key, [data](LfgCompatibleContainer::value_type& item)
    {
        item.second.compatibility = data.compatibility; item.second.roles = data.roles;
    });
}

/**
   Get the compatibility of a group of guids

   @param[in]     key String concatenation of guids (| used as separator)
   @return LfgCompatibility type of compatibility
*/
LfgCompatibility LFGQueue::GetCompatibles(std::string const& key)
{
    std::lock_guard<std::recursive_mutex> _lock(m_lock);
    LfgCompatibleContainer::guarded_ptr ptr = CompatibleMapStore.get(key);
    if (ptr)
        return ptr->second.compatibility;

    return LFG_COMPATIBILITY_PENDING;
}

LfgCompatibilityData* LFGQueue::GetCompatibilityData(std::string const& key)
{
    std::lock_guard<std::recursive_mutex> _lock(m_lock);
    LfgCompatibleContainer::guarded_ptr ptr = CompatibleMapStore.get(key);
    if (ptr)
        return &ptr->second;

    return nullptr;
}

uint8 LFGQueue::FindGroups()
{
    uint8 proposals = 0;
    GuidList firstNew;
    while (!newToQueueStore.empty())
    {
        ObjectGuid frontguid = newToQueueStore.front();
        TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGQueue::FindGroups: checking %s newToQueue(%u), currentQueue(%u)", frontguid.ToString().c_str(), uint32(newToQueueStore.size()), uint32(currentQueueStore.size()));
        firstNew.clear();
        firstNew.push_back(frontguid);
        RemoveFromNewQueue(frontguid);

        GuidList temporalList = currentQueueStore;
        LfgCompatibility compatibles = FindNewGroups(firstNew, temporalList);

        if (compatibles == LFG_COMPATIBLES_MATCH)
            ++proposals;
        else
            AddToCurrentQueue(frontguid);                  // Lfg group not found, add this group to the queue.
    }
    return proposals;
}

/**
   Checks que main queue to try to form a Lfg group. Returns first match found (if any)

   @param[in]     check List of guids trying to match with other groups
   @param[in]     all List of all other guids in main queue to match against
   @return LfgCompatibility type of compatibility between groups
*/
LfgCompatibility LFGQueue::FindNewGroups(GuidList& check, GuidList& all)
{
    if (check.empty() || check.size() > MAX_GROUP_SIZE)
        return LFG_INCOMPATIBLES_WRONG_GROUP_SIZE;

    std::string strGuids = ConcatenateGuids(check);
    LfgCompatibility compatibles = GetCompatibles(strGuids);

    // TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGQueue::FindNewGroup: (%s): %s - all(%s)", strGuids.c_str(), GetCompatibleString(compatibles), ConcatenateGuids(all).c_str());
    if (compatibles == LFG_COMPATIBILITY_PENDING) // Not previously cached, calculate
        compatibles = CheckCompatibility(check);
    // TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGQueue::FindNewGroup2: (%s): %s - all(%s)", strGuids.c_str(), GetCompatibleString(compatibles), ConcatenateGuids(all).c_str());

    if (compatibles == LFG_COMPATIBLES_BAD_STATES && sLFGMgr->AllQueued(check, queueId))
    {
        TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGQueue::FindNewGroup: (%s) compatibles (cached) changed from bad states to match", strGuids.c_str());
        SetCompatibles(strGuids, LFG_COMPATIBLES_MATCH);
        return LFG_COMPATIBLES_MATCH;
    }

    if (compatibles != LFG_COMPATIBLES_WITH_LESS_PLAYERS)
        return compatibles;

    // Try to match with queued groups
    while (!all.empty())
    {
        ObjectGuid guid = all.front();
        all.pop_front();

        if (sLFGMgr->GetState(guid, queueId) == LFG_STATE_WAITE)
            continue;

        check.push_back(guid);
        LfgCompatibility subcompatibility = FindNewGroups(check, all);
        if (subcompatibility == LFG_COMPATIBLES_MATCH)
            return LFG_COMPATIBLES_MATCH;
        check.pop_back();
    }
    return compatibles;
}

/**
   Check compatibilities between groups. If group is Matched proposal will be created

   @param[in]     check List of guids to check compatibilities
   @return LfgCompatibility type of compatibility
*/
LfgCompatibility LFGQueue::CheckCompatibility(GuidList check)
{
    std::string strGuids = ConcatenateGuids(check);
    LfgProposal proposal;
    LfgDungeonSet proposalDungeons;
    LfgGroupsMap proposalGroups;
    LfgRolesMap proposalRoles;

    proposal.queueId = queueId;

    uint8 minGroupSize = MAX_GROUP_SIZE;
    uint8 maxGroupSize = MAX_GROUP_SIZE;
    bool forceMinPlayers = sWorld->getBoolConfig(CONFIG_LFG_FORCE_MINPLAYERS) || sWorld->getBoolConfig(CONFIG_LFG_DEBUG_JOIN);
    bool noNeedWaightConfirm = false;
    LfgDungeonSet::const_iterator itr = QueueDataStore[check.front()].dungeons.begin();
    if (itr != QueueDataStore[check.front()].dungeons.end())
        if (LFGDungeonData const* dungeon = sLFGMgr->GetLFGDungeon(*itr))
        {
            minGroupSize = dungeon->dbc->GetMinGroupSize();
            maxGroupSize = dungeon->dbc->GetMaxGroupSize();
            //The Battle for Broken Shore
            if (dungeon->map == 1460)
            {
                minGroupSize = 1;
                forceMinPlayers = true;
                noNeedWaightConfirm = true;
            }
        }

    if (sWorld->getBoolConfig(CONFIG_LFG_DEBUG_JOIN))
        minGroupSize = 1;

    // Check for correct size
    if (check.size() > maxGroupSize || check.empty())
    {
        TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGQueue::CheckCompatibility: (%s): Size wrong - Not compatibles", strGuids.c_str());
        return LFG_INCOMPATIBLES_WRONG_GROUP_SIZE;
    }

    // Check all-but-new compatiblitity
    if (check.size() > 2)
    {
        ObjectGuid frontGuid = check.front();
        check.pop_front();

        // Check all-but-new compatibilities (New, A, B, C, D) --> check(A, B, C, D)
        LfgCompatibility child_compatibles = CheckCompatibility(check);
        if (child_compatibles < LFG_COMPATIBLES_WITH_LESS_PLAYERS) // Group not compatible
        {
            // TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGQueue::CheckCompatibility: (%s) child %s not compatibles", strGuids.c_str(), ConcatenateGuids(check).c_str());
            SetCompatibles(strGuids, child_compatibles);
            return child_compatibles;
        }
        check.push_front(frontGuid);
    }

    // Check if more than one LFG group and number of players joining
    uint8 numPlayers = 0;
    uint8 numLfgGroups = 0;
    for (GuidList::const_iterator it = check.begin(); it != check.end() && numLfgGroups < 2 && numPlayers <= maxGroupSize; ++it)
    {
        ObjectGuid guid = *it;
        LfgQueueDataContainer::iterator itQueue = QueueDataStore.find(guid);
        if (itQueue == QueueDataStore.end())
        {
            TC_LOG_ERROR(LOG_FILTER_LFG, "LFGQueue::CheckCompatibility: %s is not queued but listed as queued!", guid.ToString().c_str());
            RemoveFromQueue(guid);
            return LFG_COMPATIBILITY_PENDING;
        }

        // Store group so we don't need to call Mgr to get it later (if it's player group will be 0 otherwise would have joined as group)
        for (LfgRolesMap::const_iterator it2 = itQueue->second.roles.begin(); it2 != itQueue->second.roles.end(); ++it2)
            proposalGroups[it2->first] = itQueue->first.IsParty() ? itQueue->first : ObjectGuid::Empty;

        numPlayers += itQueue->second.roles.size();

        if (sLFGMgr->IsLfgGroup(guid))
        {
            if (!numLfgGroups)
                proposal.group = guid;
            ++numLfgGroups;
        }
    }

    ObjectGuid gguid = *check.begin();
    proposal.isNew = numLfgGroups != 1 || sLFGMgr->GetOldState(gguid, queueId) != LFG_STATE_DUNGEON;

    // Group with less that MAX_GROUP_SIZE members always compatible
    if (!sLFGMgr->onTest() && check.size() == 1 && numPlayers < (proposal.isNew && !forceMinPlayers ? maxGroupSize : minGroupSize))
    {
        TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGQueue::CheckCompatibility: (%s) sigle group. Compatibles", strGuids.c_str());
        LfgQueueDataContainer::iterator itQueue = QueueDataStore.find(check.front());

        LfgCompatibilityData data(LFG_COMPATIBLES_WITH_LESS_PLAYERS);
        data.roles = itQueue->second.roles;
        uint32 n = 0;
        LFGMgr::CheckGroupRoles(data.roles, LfgRoleData(*itQueue->second.dungeons.begin() & 0xFFFFF), n);

        UpdateBestCompatibleInQueue(itQueue, strGuids, data.roles);
        SetCompatibilityData(strGuids, data);
        return LFG_COMPATIBLES_WITH_LESS_PLAYERS;
    }

    if (numLfgGroups > 1)
    {
        TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGQueue::CheckCompatibility: (%s) More than one Lfggroup (%u)", strGuids.c_str(), numLfgGroups);
        SetCompatibles(strGuids, LFG_INCOMPATIBLES_MULTIPLE_LFG_GROUPS);
        return LFG_INCOMPATIBLES_MULTIPLE_LFG_GROUPS;
    }

    if (numPlayers > maxGroupSize)
    {
        TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGQueue::CheckCompatibility: (%s) Too much players (%u)", strGuids.c_str(), numPlayers);
        SetCompatibles(strGuids, LFG_INCOMPATIBLES_TOO_MUCH_PLAYERS);
        return LFG_INCOMPATIBLES_TOO_MUCH_PLAYERS;
    }

    // If it's single group no need to check for duplicate players, ignores, bad roles or bad dungeons as it's been checked before joining
    if (check.size() > 1)
    {
        for (GuidList::const_iterator it = check.begin(); it != check.end(); ++it)
        {
            if (QueueDataStore.find(*it) == QueueDataStore.end())
                TC_LOG_ERROR(LOG_FILTER_LFG, "LFGQueue::CheckCompatibility: ERROR! player queue data not found! 1");

            const LfgRolesMap &roles = QueueDataStore[(*it)].roles;
            for (LfgRolesMap::const_iterator itRoles = roles.begin(); itRoles != roles.end(); ++itRoles)
            {
                LfgRolesMap::const_iterator itPlayer;
                for (itPlayer = proposalRoles.begin(); itPlayer != proposalRoles.end(); ++itPlayer)
                {
                    if (itRoles->first == itPlayer->first)
                        TC_LOG_ERROR(LOG_FILTER_LFG, "LFGQueue::CheckCompatibility: ERROR! Player multiple times in queue! %s", itRoles->first.ToString().c_str());
                    else if (sLFGMgr->HasIgnore(itRoles->first, itPlayer->first))
                        break;
                }
                if (itPlayer == proposalRoles.end())
                    proposalRoles[itRoles->first] = itRoles->second;
            }
        }

        if (uint8 playersize = numPlayers - proposalRoles.size())
        {
            TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGQueue::CheckCompatibility: (%s) not compatible, %u players are ignoring each other", strGuids.c_str(), playersize);
            SetCompatibles(strGuids, LFG_INCOMPATIBLES_HAS_IGNORES);
            return LFG_INCOMPATIBLES_HAS_IGNORES;
        }

        GuidList::iterator itguid = check.begin();
        if (QueueDataStore.find(*itguid) == QueueDataStore.end())
            TC_LOG_ERROR(LOG_FILTER_LFG, "LFGQueue::CheckCompatibility: ERROR! player queue data not found! 2");

        proposalDungeons = QueueDataStore[*itguid].dungeons;
        LfgRolesMap debugRoles = proposalRoles;
        uint32 n = 0;
        if (!LFGMgr::CheckGroupRoles(proposalRoles, LfgRoleData(*proposalDungeons.begin() & 0xFFFFF), n))
        {
            std::ostringstream o;
            for (LfgRolesMap::const_iterator it = debugRoles.begin(); it != debugRoles.end(); ++it)
                o << ", " << it->first << ": " << GetRolesString(it->second);

            TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGQueue::CheckCompatibility: (%s) Roles not compatible%s", strGuids.c_str(), o.str().c_str());
            SetCompatibles(strGuids, LFG_INCOMPATIBLES_NO_ROLES);
            return LFG_INCOMPATIBLES_NO_ROLES;
        }

        std::ostringstream o;
        o << ", " << *itguid << ": (" << ConcatenateDungeons(proposalDungeons) << ")";
        for (++itguid; itguid != check.end(); ++itguid)
        {
            LfgDungeonSet temporal;
            LfgDungeonSet &dungeons = QueueDataStore[*itguid].dungeons;
            o << ", " << *itguid << ": (" << ConcatenateDungeons(dungeons) << ")";
            std::set_intersection(proposalDungeons.begin(), proposalDungeons.end(), dungeons.begin(), dungeons.end(), std::inserter(temporal, temporal.begin()));
            proposalDungeons = temporal;
        }

        if (proposalDungeons.empty())
        {
            TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGQueue::CheckCompatibility: (%s) No compatible dungeons%s", strGuids.c_str(), o.str().c_str());
            SetCompatibles(strGuids, LFG_INCOMPATIBLES_NO_DUNGEONS);
            return LFG_INCOMPATIBLES_NO_DUNGEONS;
        }
    }
    else
    {
        ObjectGuid guid2 = *check.begin();
        const LfgQueueData &queue = QueueDataStore[guid2];
        proposalDungeons = queue.dungeons;
        proposalRoles = queue.roles;
        uint32 n = 0;
        LFGMgr::CheckGroupRoles(proposalRoles, LfgRoleData(*proposalDungeons.begin() & 0xFFFFF), n);       // assing new roles
    }

    // Enough players?
    if (!sLFGMgr->onTest() && numPlayers < (proposal.isNew && !forceMinPlayers ? maxGroupSize : minGroupSize))
    {
        TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGQueue::CheckCompatibility: (%s) Compatibles but not enough players(%u) (%u/%u)", strGuids.c_str(), numPlayers, minGroupSize, maxGroupSize);

        LfgCompatibilityData data(LFG_COMPATIBLES_WITH_LESS_PLAYERS);
        data.roles = proposalRoles;

        for (GuidList::const_iterator itr2 = check.begin(); itr2 != check.end(); ++itr2)
            UpdateBestCompatibleInQueue(QueueDataStore.find(*itr2), strGuids, data.roles);

        SetCompatibilityData(strGuids, data);
        return LFG_COMPATIBLES_WITH_LESS_PLAYERS;
    }

    proposal.queues = check;

    if (!sLFGMgr->AllQueued(check, queueId))
    {
        TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGQueue::CheckCompatibility: (%s) Group MATCH but can't create proposal!", strGuids.c_str());
        SetCompatibles(strGuids, LFG_COMPATIBLES_BAD_STATES);
        return LFG_COMPATIBLES_BAD_STATES;
    }

    // Create a new proposal
    proposal.cancelTime = time(nullptr) + LFG_TIME_PROPOSAL;
    proposal.leader.Clear();
    proposal.dungeonId = Trinity::Containers::SelectRandomContainerElement(proposalDungeons);
    proposal.encounters = sLFGMgr->GetCompletedMask(gguid);

    bool leader = false;
    bool allAccepted = true;
    for (LfgRolesMap::const_iterator itRoles = proposalRoles.begin(); itRoles != proposalRoles.end(); ++itRoles)
    {
        // Assing new leader
        if (itRoles->second & PLAYER_ROLE_LEADER)
        {
            if (!leader || !proposal.leader || urand(0, 1))
                proposal.leader = itRoles->first;
            leader = true;
        }
        else if (!leader && (!proposal.leader || urand(0, 1)))
            proposal.leader = itRoles->first;

        // Assing player data and roles
        LfgProposalPlayer &data = proposal.players[itRoles->first];
        data.role = itRoles->second;
        data.group = proposalGroups.find(itRoles->first)->second;
        if (!proposal.isNew && !data.group.IsEmpty() && data.group == proposal.group) // Player from existing group, autoaccept
            data.accept = LFG_ANSWER_AGREE;

        if (noNeedWaightConfirm/*possible some flag do it*/)
            data.accept = LFG_ANSWER_AGREE;

        allAccepted &= data.accept == LFG_ANSWER_AGREE;
    }

    if (allAccepted)
        proposal.state = LFG_PROPOSAL_SUCCESS;
    else
        proposal.state = LFG_PROPOSAL_INITIATING;

    // Mark proposal members as not queued (but not remove queue data)
    for (GuidList::const_iterator itQueue = proposal.queues.begin(); itQueue != proposal.queues.end(); ++itQueue)
    {
        ObjectGuid guid = *itQueue;
        RemoveFromNewQueue(guid);
        RemoveFromCurrentQueue(guid);
    }

    sLFGMgr->AddProposal(proposal);

    TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGQueue::CheckCompatibility: (%s) MATCH! Group formed", strGuids.c_str());
    SetCompatibles(strGuids, LFG_COMPATIBLES_MATCH);
    return LFG_COMPATIBLES_MATCH;
}

void LFGQueue::UpdateQueueTimers(time_t currTime)
{
    TC_LOG_TRACE(LOG_FILTER_LFG, "Updating queue timers...");
    for (LfgQueueDataContainer::iterator itQueue = QueueDataStore.begin(); itQueue != QueueDataStore.end(); ++itQueue)
    {
        LfgQueueData& queueinfo = itQueue->second;
        if (queueinfo.dungeons.empty())
            continue;

        uint32 dungeonId = *queueinfo.dungeons.begin();
        if (!itQueue->first.IsParty())
        {
            LfgDungeonSet const& dungeons = sLFGMgr->GetSelectedDungeons(itQueue->first, queueId);
            if (!dungeons.empty())
                dungeonId = *dungeons.begin();
        }
        else
        {
            LfgRolesMap::const_iterator itPlayer = queueinfo.roles.begin();
            if (itPlayer != queueinfo.roles.end())
            {
                LfgDungeonSet const& dungeons = sLFGMgr->GetSelectedDungeons(itPlayer->first, queueId);
                if (!dungeons.empty())
                    dungeonId = *dungeons.begin();
            }

        }

        uint32 queuedTime = uint32(currTime - queueinfo.joinTime);
        uint8 role = PLAYER_ROLE_NONE;
        int32 waitTime = -1;
        int32 wtTank = waitTimesTankStore[dungeonId].time;
        int32 wtHealer = waitTimesHealerStore[dungeonId].time;
        int32 wtDps = waitTimesDpsStore[dungeonId].time;
        int32 wtAvg = waitTimesAvgStore[dungeonId].time;

        for (LfgRolesMap::const_iterator itPlayer = queueinfo.roles.begin(); itPlayer != queueinfo.roles.end(); ++itPlayer)
            role |= itPlayer->second;
        role &= ~PLAYER_ROLE_LEADER;

        switch (role)
        {
            case PLAYER_ROLE_NONE:                                // Should not happen - just in case
                waitTime = -1;
                break;
            case PLAYER_ROLE_TANK:
                waitTime = wtTank;
                break;
            case PLAYER_ROLE_HEALER:
                waitTime = wtHealer;
                break;
            case PLAYER_ROLE_DAMAGE:
                waitTime = wtDps;
                break;
            default:
                waitTime = wtAvg;
                break;
        }

        if (queueinfo.bestCompatible.empty())
            FindBestCompatibleInQueue(itQueue);

        LfgQueueStatusData queueData(dungeonId, waitTime, wtAvg, wtTank, wtHealer, wtDps, queuedTime, &queueinfo);
        for (LfgRolesMap::const_iterator itPlayer = queueinfo.roles.begin(); itPlayer != queueinfo.roles.end(); ++itPlayer)
            sLFGMgr->SendLfgQueueStatus(itPlayer->first, queueData);
    }
}

LfgQueueData const* LFGQueue::GetQueueData(ObjectGuid guid)
{
    return Trinity::Containers::MapGetValuePtr(QueueDataStore, guid);
}

time_t LFGQueue::GetJoinTime(ObjectGuid guid)
{
    LfgQueueDataContainer::const_iterator itr = QueueDataStore.find(guid);
    return itr != QueueDataStore.end() ? itr->second.joinTime : time(nullptr);
}

uint8 LFGQueue::GetQueueType(ObjectGuid guid)
{
    LfgQueueDataContainer::const_iterator itr = QueueDataStore.find(guid);
    return itr != QueueDataStore.end() ? itr->second.type : LFG_TYPE_DUNGEON;
}

uint8 LFGQueue::GetQueueSubType(ObjectGuid guid)
{
    LfgQueueDataContainer::const_iterator itr = QueueDataStore.find(guid);
    return itr != QueueDataStore.end() ? itr->second.subType : LFG_QUEUE_DUNGEON;
}

LfgQueueData::LfgQueueData() : joinTime(time_t(time(nullptr))), type(LFG_TYPE_DUNGEON), subType(LFG_QUEUE_DUNGEON)
{
    tanks = tanksNeeded = minTanksNeeded = LFG_TANKS_NEEDED;
    healers = healerNeeded = minHealerNeeded = LFG_HEALERS_NEEDED;
    dps = dpsNeeded = minDpsNeeded = LFG_DPS_NEEDED;
}

LfgQueueData::LfgQueueData(time_t _joinTime, LfgDungeonSet const& _dungeons, const LfgRolesMap &_roles)
{
    LFGDungeonData const* dungeon = !_dungeons.empty() ? sLFGMgr->GetLFGDungeon(*_dungeons.begin() & 0xFFFFF) : nullptr;
    type = dungeon ? dungeon->internalType : LFG_TYPE_DUNGEON;
    subType = dungeon ? dungeon->dbc->Substruct : LFG_QUEUE_DUNGEON;
    joinTime = _joinTime;
    dungeons = _dungeons;
    roles = _roles;

    minTanksNeeded = dungeon ? dungeon->dbc->MinCountTank : LFG_TANKS_NEEDED;
    minHealerNeeded = dungeon ? dungeon->dbc->MinCountHealer : LFG_HEALERS_NEEDED;
    minDpsNeeded = dungeon ? dungeon->dbc->MinCountDamage : LFG_DPS_NEEDED;

    tanksNeeded = dungeon ? dungeon->dbc->CountTank : LFG_TANKS_NEEDED;
    healerNeeded = dungeon ? dungeon->dbc->CountHealer : LFG_HEALERS_NEEDED;
    dpsNeeded = dungeon ? dungeon->dbc->CountDamage : LFG_DPS_NEEDED;

    tanks = tanksNeeded;
    healers = healerNeeded;
    dps = dpsNeeded;
}

std::string LFGQueue::DumpQueueInfo() const
{
    uint32 players = 0;
    uint32 groups = 0;
    uint32 playersInGroup = 0;

    for (uint8 i = 0; i < 2; ++i)
    {
        GuidList const& queue = i ? newToQueueStore : currentQueueStore;
        for (GuidList::const_iterator it = queue.begin(); it != queue.end(); ++it)
        {
            ObjectGuid guid = *it;
            if (guid.IsParty())
            {
                groups++;
                playersInGroup += sLFGMgr->GetPlayerCount(guid);
            }
            else
                players++;
        }
    }
    std::ostringstream o;
    o << "Queued Players: " << players << " (in group: " << playersInGroup << ") Groups: " << groups << "\n";
    return o.str();
}

std::string LFGQueue::DumpCompatibleInfo(bool full /* = false */) const
{
    std::ostringstream o;
    o << "Compatible Map size: " << CompatibleMapStore.size() << "\n";
    if (full)
        for (LfgCompatibleContainer::const_iterator itr = CompatibleMapStore.begin(); itr != CompatibleMapStore.end(); ++itr)
            o << "(" << itr->first << "): " << GetCompatibleString(itr->second.compatibility) << "\n";

    return o.str();
}

void LFGQueue::FindBestCompatibleInQueue(LfgQueueDataContainer::iterator itrQueue)
{
    std::lock_guard<std::recursive_mutex> _lock(m_lock);

    TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGQueue::FindBestCompatibleInQueue: %s", itrQueue->first.ToString().c_str());
    std::ostringstream o;
    o << itrQueue->first;
    std::string sguid = o.str();

    for (LfgCompatibleContainer::iterator itr = CompatibleMapStore.begin(); itr != CompatibleMapStore.end(); ++itr)
        if (itr->second.compatibility == LFG_COMPATIBLES_WITH_LESS_PLAYERS && std::string::npos != itr->first.find(sguid))
            UpdateBestCompatibleInQueue(itrQueue, itr->first, itr->second.roles);
}

void LFGQueue::UpdateBestCompatibleInQueue(LfgQueueDataContainer::iterator itrQueue, std::string const& key, LfgRolesMap const& roles)
{
    LfgQueueData& queueData = itrQueue->second;

    uint8 storedSize = queueData.bestCompatible.empty() ? 0 :
        std::count(queueData.bestCompatible.begin(), queueData.bestCompatible.end(), '|') + 1;

    uint8 size = std::count(key.begin(), key.end(), '|') + 1;

    if (size <= storedSize)
        return;

    TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGQueue::UpdateBestCompatibleInQueue: Changed (%s) to (%s) as best compatible group for %s",
        queueData.bestCompatible.c_str(), key.c_str(), itrQueue->first.ToString().c_str());

    queueData.bestCompatible = key;
    queueData.tanks = queueData.tanksNeeded;
    queueData.healers = queueData.healerNeeded;
    queueData.dps = queueData.dpsNeeded;

    for (LfgRolesMap::const_iterator it = roles.begin(); it != roles.end(); ++it)
    {
        uint8 role = it->second;
        if (role & PLAYER_ROLE_TANK)
        {
            if (queueData.tanks)
                --queueData.tanks;
        }
        else if (role & PLAYER_ROLE_HEALER)
        {
            if (queueData.healers)
                --queueData.healers;
        }
        else if (role & PLAYER_ROLE_DAMAGE)
        {
            if (queueData.dps)
                --queueData.dps;
        }
    }
}

void LFGQueue::UpdateShortageData()
{
    // Collect all queued players' roles for shortage calculation
    uint32 tanksCount = 0, healersCount = 0, dpsCount = 0;
    for (GuidList::const_iterator itr = currentQueueStore.begin(); itr != currentQueueStore.end(); ++itr)
    {
        ObjectGuid guid = *itr;
        if (sLFGMgr->GetState(guid, queueId) != LFG_STATE_QUEUED)
            continue;
        if (guid.IsParty())
        {
            GuidSet const& members = sLFGMgr->GetPlayers(guid);
            for (GuidSet::const_iterator mitr = members.begin(); mitr != members.end(); ++mitr)
            {
                uint8 roles = sLFGMgr->GetRoles(*mitr, queueId);
                if (roles & PLAYER_ROLE_TANK)
                    ++tanksCount;
                if (roles & PLAYER_ROLE_HEALER)
                    ++healersCount;
                if (roles & PLAYER_ROLE_DAMAGE)
                    ++dpsCount;
            }
        }
        else
        {
            uint8 roles = sLFGMgr->GetRoles(guid, queueId);
            if (roles & PLAYER_ROLE_TANK)
                ++tanksCount;
            if (roles & PLAYER_ROLE_HEALER)
                ++healersCount;
            if (roles & PLAYER_ROLE_DAMAGE)
                ++dpsCount;
        }
    }
    ShortageData = LfgShortageData(tanksCount, healersCount, dpsCount, queueId);
}

uint8 LFGQueue::GetShortageRoles()
{
    return ShortageData.GetShortageRoles();
}

uint8 LFGQueue::IsEligibleForCTAReward(uint8 roles)
{
    uint8 shortageRoles = ShortageData.GetShortageRoles();
    if (!shortageRoles)
        return 0;
    return shortageRoles & roles;
}

uint8 LfgShortageData::GetShortageRoles()
{
    if (shortageRolesCalculated)
        return shortageRoles;

    LFGDungeonData const* dungeonData = sLFGMgr->GetLFGDungeon(queueId);
    if (!dungeonData || !dungeonData->dbc)
        return shortageRoles;

    const float idealTanksRatio = dungeonData->dbc->MinCountTank * 1.0f / dungeonData->dbc->GetMinGroupSize() * 1.0f;
    const float idealHealersRatio = dungeonData->dbc->MinCountHealer * 1.0f / dungeonData->dbc->GetMinGroupSize() * 1.0f;
    const float idealDPSRatio = dungeonData->dbc->MinCountDamage * 1.0f / dungeonData->dbc->GetMinGroupSize() * 1.0f;

    uint32 total = totalTanks + totalHealers + totalDps;
    if (!total)
        return shortageRoles;

    float tanksRatio = (float)totalTanks / total;
    float healersRatio = (float)totalHealers / total;
    float dpsRatio = (float)totalDps / total;

    if (tanksRatio < idealTanksRatio * GetShortagePercent())
        shortageRoles |= PLAYER_ROLE_TANK;
    if (healersRatio < idealHealersRatio * GetShortagePercent())
        shortageRoles |= PLAYER_ROLE_HEALER;
    if (dpsRatio < idealDPSRatio * GetShortagePercent())
        shortageRoles |= PLAYER_ROLE_DAMAGE;

    shortageRolesCalculated = true;

    return shortageRoles;
}

} // namespace lfg
