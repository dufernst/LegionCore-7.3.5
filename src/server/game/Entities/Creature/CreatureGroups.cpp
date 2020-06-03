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

#include "Creature.h"
#include "CreatureGroups.h"
#include "ObjectMgr.h"
#include "DatabaseEnv.h"
#include "CreatureAI.h"

#define MAX_DESYNC 5.0f

FormationMgr::~FormationMgr()
{
    for (auto& itr : CreatureGroupMap)
        delete itr.second;
}

FormationMgr* FormationMgr::instance()
{
    static FormationMgr instance;
    return &instance;
}

void FormationMgr::AddCreatureToGroup(ObjectGuid::LowType const& groupId, Creature* member)
{
    Map* map = member->FindMap();
    if (!map)
        return;

    auto itr = map->CreatureGroupHolder.find(groupId);

    //Add member to an existing group
    if (itr != map->CreatureGroupHolder.end())
    {
        TC_LOG_DEBUG(LOG_FILTER_UNITS, "Group found: %u, inserting creature GUID: %u, Group InstanceID %u", groupId, member->GetGUID().GetGUIDLow(), member->GetInstanceId());
        itr->second->AddMember(member);
    }
    //Create new group
    else
    {
        map->_creatureGroupLock.lock();
        TC_LOG_DEBUG(LOG_FILTER_UNITS, "Group not found: %u. Creating new group.", groupId);
        auto group = new CreatureGroup(groupId);
        map->CreatureGroupHolder[groupId] = group;
        group->AddMember(member);
        map->_creatureGroupLock.unlock();
    }
}

void FormationMgr::RemoveCreatureFromGroup(CreatureGroup* group, Creature* member)
{
    TC_LOG_DEBUG(LOG_FILTER_UNITS, "Deleting member pointer to GUID: %u from group %u", group->GetId(), member->GetDBTableGUIDLow());
    group->RemoveMember(member);

    if (group->isEmpty())
    {
        Map* map = member->FindMap();
        if (!map)
            return;

        map->_creatureGroupLock.lock();
        TC_LOG_DEBUG(LOG_FILTER_UNITS, "Deleting group with InstanceID %u", member->GetInstanceId());
        map->CreatureGroupHolder.erase(group->GetId());
        map->_creatureGroupLock.unlock();
        delete group;
    }
}

void FormationMgr::LoadCreatureFormations()
{
    uint32 oldMSTime = getMSTime();

    for (auto& itr : CreatureGroupMap) // for reload case
        delete itr.second;
    CreatureGroupMap.clear();

    //Get group data
    QueryResult result = WorldDatabase.Query("SELECT leaderGUID, memberGUID, dist, angle, groupAI FROM creature_formations ORDER BY leaderGUID");

    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">>  Loaded 0 creatures in formations. DB table `creature_formations` is empty!");
        return;
    }

    uint32 count = 0;

    do
    {
        Field * fields = result->Fetch();

        //Load group member data
        auto group_member = new FormationInfo();
        group_member->leaderGUID            = fields[0].GetUInt64();
        ObjectGuid::LowType memberGUID      = fields[1].GetUInt64();
        group_member->groupAI               = fields[4].GetUInt32() == 515 ? 5 : fields[4].GetUInt32();  // fucking TK =C
        //If creature is group leader we may skip loading of dist/angle
        if (group_member->leaderGUID != memberGUID)
        {
            group_member->follow_dist       = fields[2].GetFloat();
            group_member->follow_angle      = fields[3].GetFloat() * M_PI / 180;
        }
        else
        {
            group_member->follow_dist       = 0;
            group_member->follow_angle      = 0;
        }

        // check data correctness
        {
            if (!sObjectMgr->GetCreatureData(group_member->leaderGUID))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "creature_formations table leader guid %u incorrect (not exist)", group_member->leaderGUID);
                delete group_member;
                continue;
            }

            if (!sObjectMgr->GetCreatureData(memberGUID))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "creature_formations table member guid %u incorrect (not exist)", memberGUID);
                delete group_member;
                continue;
            }
        }

        CreatureGroupMap[memberGUID] = group_member;
        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u creatures in formations in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

FormationInfo* FormationMgr::CreateCustomFormation(Creature* c)
{
    auto group_member         = new FormationInfo();
    group_member->leaderGUID            = c->GetGUIDLow();
    group_member->groupAI               = 2;
    group_member->follow_dist           = 1.0f;
    group_member->follow_angle          = 45 * M_PI / 180;
    sFormationMgr->AddCreatureToGroup(c->GetGUIDLow(), c);
    if (CreatureGroup* f = c->GetFormation())
        f->AddMember(c, group_member);
    return group_member;
}

CreatureGroup::CreatureGroup(ObjectGuid::LowType const& id): m_leader(nullptr), m_groupID(id), m_Formed(false)
{
}

void CreatureGroup::AddMember(Creature* member, FormationInfo* f)
{
    TC_LOG_DEBUG(LOG_FILTER_UNITS, "CreatureGroup::AddMember: Adding unit GetDBTableGUIDLow %u GUID: %u", member->GetDBTableGUIDLow(), member->GetGUID().GetGUIDLow());

    //Check if it is a leader
    if (member->GetDBTableGUIDLow() == m_groupID || member->GetGUIDLow() == m_groupID)
    {
        TC_LOG_DEBUG(LOG_FILTER_UNITS, "Unit GUID: %u is formation leader. Adding group.", member->GetGUID().GetGUIDLow());
        m_leader = member;
    }

    auto itr = sFormationMgr->CreatureGroupMap.find(member->GetDBTableGUIDLow());
    if (itr != sFormationMgr->CreatureGroupMap.end())
        m_members[member] = itr->second;
    else if (f)
        m_members[member] = f;

    member->SetFormation(this);
}

void CreatureGroup::RemoveMember(Creature* member)
{
    TC_LOG_DEBUG(LOG_FILTER_UNITS, "CreatureGroup::RemoveMember: Removing unit GetDBTableGUIDLow %u GUID: %u entry %u", member->GetDBTableGUIDLow(), member->GetGUID().GetGUIDLow(), member->GetEntry());

    if (m_leader == member)
        m_leader = nullptr;

    m_members.erase(member);
    member->SetFormation(nullptr);
}

void CreatureGroup::MemberAttackStart(Creature* member, Unit* target)
{
    if(!member || !target)
        return;

    #ifdef WIN32
    if(m_leader)
        TC_LOG_DEBUG(LOG_FILTER_UNITS, "CreatureGroup::MemberAttackStart: GetDBTableGUIDLow %u GetGUIDLow %u entry %u m_leader %u %u", member->GetDBTableGUIDLow(), member->GetGUIDLow(), member->GetEntry(), m_leader->GetEntry(), m_leader->GetGUIDLow());
    #endif

    uint8 groupAI = 0;

    auto itr = m_members.find(member);
    if (itr != m_members.end() && (*itr).second)
        groupAI = (*itr).second->groupAI;
    else if (member->GetDBTableGUIDLow())
    {
        auto itr = sFormationMgr->CreatureGroupMap.find(member->GetDBTableGUIDLow());
        if (itr != sFormationMgr->CreatureGroupMap.end())
            groupAI = itr->second->groupAI;
        else
            groupAI = 2;
    }
    else
        groupAI = 2;

    if (!groupAI)
        return;

    if (groupAI == 1 && member != m_leader)
        return;

    for (auto& itr : m_members)
    {
        if (m_leader) // avoid crash if leader was killed and reset.
            TC_LOG_DEBUG(LOG_FILTER_UNITS, "GROUP ATTACK: group instance id %u calls member instid %u", m_leader->GetInstanceId(), member->GetInstanceId());

        //Skip one check
        if (itr.first == member)
            continue;

        if (!itr.first->isAlive() || !itr.first->IsInWorld())
            continue;

        if (itr.first->getVictim())
            continue;

        if (itr.first->IsInEvadeMode())
            continue;

        if (itr.first->AI() && itr.first->canStartAttack(target, true))
            itr.first->AI()->AttackStart(target);
    }
}

void CreatureGroup::FormationReset(bool dismiss)
{
    for (auto& itr : m_members)
    {
        if (itr.first != m_leader && itr.first->isAlive() && itr.first->IsInWorld())
        {
            if (dismiss)
                itr.first->GetMotionMaster()->Initialize();
            else
                itr.first->GetMotionMaster()->MoveIdle();

            TC_LOG_DEBUG(LOG_FILTER_UNITS, "Set %s movement for member GUID: %u", dismiss ? "default" : "idle",itr.first->GetGUIDLow());
        }
    }
    m_Formed = !dismiss;
}

void CreatureGroup::LeaderMoveTo(float x, float y, float z)
{
    //! To do: This should probably get its own movement generator or use WaypointMovementGenerator.
    //! If the leader's path is known, member's path can be plotted as well using formation offsets.
    if (!m_leader)
        return;

    float pathangle = atan2(m_leader->GetPositionY() - y, m_leader->GetPositionX() - x);

    for (auto& itr : m_members)
    {
        Creature* member = itr.first;
        if (member == m_leader || !member->isAlive() || member->getVictim() || !member->IsInWorld())
            continue;

        if (member->IsInEvadeMode())
            continue;

        uint8 groupAI = 0;
        auto mitr = m_members.find(member);
        if (mitr != m_members.end() && (*mitr).second)
            groupAI = (*mitr).second->groupAI;
        else if (member->GetDBTableGUIDLow())
        {
            auto itr = sFormationMgr->CreatureGroupMap.find(member->GetDBTableGUIDLow());
            if (itr != sFormationMgr->CreatureGroupMap.end())
                groupAI = itr->second->groupAI;
            else
                groupAI = 2;
        }
        else
            groupAI = 2;

        if (!groupAI || groupAI == 3)
            continue;

        float angle = itr.second->follow_angle;
        float dist = itr.second->follow_dist;

        float dx = x +  std::cos(angle + pathangle) * dist;
        float dy = y +  std::sin(angle + pathangle) * dist;
        float dz = z;

        Trinity::NormalizeMapCoord(dx);
        Trinity::NormalizeMapCoord(dy);

        if (!member->CanFly())
            member->UpdateGroundPositionZ(dx, dy, dz);

        if (member->IsWithinDist(m_leader, dist + MAX_DESYNC))
            member->SetUnitMovementFlags(m_leader->GetUnitMovementFlags());
        else
            member->SetWalk(false);

        member->GetMotionMaster()->MovePoint(0, dx, dy, dz);
        member->SetHomePosition(dx, dy, dz, pathangle);
    }
}
