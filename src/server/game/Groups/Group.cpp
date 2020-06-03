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

#include "Common.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Player.h"
#include "World.h"
#include "ObjectMgr.h"
#include "GroupMgr.h"
#include "Group.h"
#include <utility>
#include "Formulas.h"
#include "ObjectAccessor.h"
#include "Battleground.h"
#include "BattlegroundMgr.h"
#include "MapManager.h"
#include "InstanceSaveMgr.h"
#include "Util.h"
#include "LFGMgr.h"
#include "UpdateFieldFlags.h"
#include "GuildMgr.h"
#include "Bracket.h"
#include "LootPackets.h"
#include "PartyPackets.h"
#include "ItemPackets.h"
#include "LootMgr.h"
#include "LFGListMgr.h"
#include "FunctionProcessor.h"
#include "DatabaseEnv.h"
#include "PlayerDefines.h"

Roll::Roll(ObjectGuid _guid, LootItem const& li) : itemCount(li.count), totalPlayersRolling(0), totalNeed(0), totalGreed(0), totalPass(0), itemSlot(0), aoeSlot(0), rollVoteMask(ROLL_ALL_TYPE_NO_DISENCHANT)
{
    item.itemGUID = _guid;
    item.ItemID = li.item.ItemID;
    item.RandomPropertiesID = li.item.RandomPropertiesID;
    item.RandomPropertiesSeed = li.item.RandomPropertiesSeed;
    item.ItemBonus.Context = li.item.ItemBonus.Context;
    item.ItemBonus.BonusListIDs = li.item.ItemBonus.BonusListIDs;
}

Roll::~Roll() = default;

Loot* Roll::getLoot()
{
    return sLootMgr->GetLoot(lootedGUID);
}

uint8 Roll::TotalEmited() const
{
    return totalNeed + totalGreed + totalPass;
}

bool Roll::isValid()
{
    return getLoot() != nullptr;
}

void Roll::FillPacket(WorldPackets::Loot::LootItem& lootItem) const
{
    lootItem.Type = LOOT_ITEM_TYPE_ITEM;
    lootItem.UIType = totalPlayersRolling > totalNeed + totalGreed + totalPass ? 1 : 0;
    lootItem.Quantity = itemCount;
    lootItem.LootListID = itemSlot + 1;
    lootItem.LootItemType = LOOT_ITEM_TYPE_ITEM;
    if (auto const& lootItemInSlot = const_cast<Roll*>(this)->getLoot()->GetItemInSlot(itemSlot))
    {
        lootItem.CanTradeToTapList = lootItemInSlot->allowedGUIDs.size() > 1;
        lootItem.Loot.Initialize(*lootItemInSlot);
    }
}

InstanceGroupBind::InstanceGroupBind() : save(nullptr), perm(false)
{
}

RaidMarker::RaidMarker(uint32 mapId, float positionX, float positionY, float positionZ, ObjectGuid transportGuid)
{
    Location.WorldRelocate(mapId, positionX, positionY, positionZ);
    TransportGUID = transportGuid;
}

Group::Group(): m_challengeInstanceID(0)
{
    m_leaderName = "";
    m_groupFlags = GROUP_FLAG_NONE;
    m_dungeonDifficulty = DIFFICULTY_NORMAL;
    m_raidDifficulty = DIFFICULTY_10_N;
    m_legacyRaidDifficulty = DIFFICULTY_10_N;
    m_bgGroup = nullptr;
    m_bfGroup = nullptr;
    m_lootMethod = PERSONAL_LOOT;
    m_lootThreshold = ITEM_QUALITY_UNCOMMON;
    m_subGroupsCounts = nullptr;
    m_maxEnchantingLevel = 0;
    m_dbStoreId = 0;
    m_readyCheckCount = 0;
    m_readyCheck = false;
    m_aoe_slots = 0;
    m_activeMarkers = 0;
    m_groupCategory = GROUP_CATEGORY_HOME;
    _team = 0;
    m_challengeEntry = nullptr;
    m_challengeLevel = 0;
    m_affixes.fill(0);
    m_dungeon = nullptr;

    for (auto& itr : m_targetIcons)
        itr.Clear();

    for (uint8 i = 0; i < RAID_MARKERS_COUNT; ++i)
        m_markers[i] = nullptr;
}

Group::~Group()
{
    if (m_bgGroup)
    {
        TC_LOG_DEBUG(LOG_FILTER_BATTLEGROUND, "Group::~Group: battleground group being deleted.");
        if (m_bgGroup->GetBgRaid(ALLIANCE) == this)
            m_bgGroup->SetBgRaid(ALLIANCE, nullptr);
        else if (m_bgGroup->GetBgRaid(HORDE) == this)
            m_bgGroup->SetBgRaid(HORDE, nullptr);
        else
            TC_LOG_ERROR(LOG_FILTER_GENERAL, "Group::~Group: battleground group is not linked to the correct battleground.");
    }

    while (!RollId.empty())
    {
        auto itr = RollId.begin();
        Roll *r = *itr;
        RollId.erase(itr);
        delete r;
    }

    // it is undefined whether objectmgr (which stores the groups) or instancesavemgr
    // will be unloaded first so we must be prepared for both cases
    // this may unload some instance saves
    for (auto& itr : m_boundInstances)
        for (auto& itr2 : itr)
            itr2.second.save->RemoveGroup(this);

    // Sub group counters clean up
    delete[] m_subGroupsCounts;
}

void Group::Update(uint32 diff)
{
    // m_Functions.Update(diff);
}

bool Group::Create(Player* leader, uint8 subType /*= 0*/, bool isLfg /*= false*/)
{
    ObjectGuid leaderGuid = leader->GetGUID();

    m_guid = ObjectGuid::Create<HighGuid::Party>(sGroupMgr->GenerateGroupId(), subType);
    m_leaderGuid = leaderGuid;
    m_leaderName = leader->GetName();
    leader->SetFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_GROUP_LEADER);

    m_groupFlags = GROUP_FLAG_NONE;

    if (isBGGroup() || isBFGroup())
    {
        m_groupFlags = GROUP_MASK_BGRAID;
        m_groupCategory = GROUP_CATEGORY_INSTANCE;
    }

    if (isLfg)
    {
        m_groupFlags = GroupFlags(m_groupFlags | GROUP_FLAG_LFG | GROUP_FLAG_LFG_RESTRICTED);
        m_groupCategory = GROUP_CATEGORY_INSTANCE;
    }

    if (m_groupFlags & GROUP_FLAG_RAID)
        _initRaidSubGroupsCounter();

    m_lootMethod = PERSONAL_LOOT;
    m_lootThreshold = ITEM_QUALITY_UNCOMMON;
    m_looterGuid = leaderGuid;

    m_dungeonDifficulty = DIFFICULTY_NORMAL;
    m_raidDifficulty = DIFFICULTY_NORMAL_RAID;
    m_legacyRaidDifficulty = DIFFICULTY_10_N;

    _team = leader->GetTeam();

    if (!isBGGroup() && !isBFGroup())
    {
        m_dungeonDifficulty = leader->GetDungeonDifficultyID();
        m_raidDifficulty = leader->GetRaidDifficultyID();
        m_legacyRaidDifficulty = leader->GetLegacyRaidDifficultyID();

        m_dbStoreId = sGroupMgr->GenerateNewGroupDbStoreId();

        sGroupMgr->RegisterGroupDbStoreId(m_dbStoreId, this);

        // Store group in database
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_GROUP);

        uint8 index = 0;

        stmt->setUInt32(index++, m_dbStoreId);
        stmt->setUInt64(index++, GetLeaderGUID().GetCounter());
        stmt->setUInt8(index++, uint8(m_lootMethod));
        stmt->setUInt64(index++, m_looterGuid.GetCounter());
        stmt->setUInt8(index++, uint8(m_lootThreshold));
        stmt->setBinary(index++, m_targetIcons[0].GetRawValue());
        stmt->setBinary(index++, m_targetIcons[1].GetRawValue());
        stmt->setBinary(index++, m_targetIcons[2].GetRawValue());
        stmt->setBinary(index++, m_targetIcons[3].GetRawValue());
        stmt->setBinary(index++, m_targetIcons[4].GetRawValue());
        stmt->setBinary(index++, m_targetIcons[5].GetRawValue());
        stmt->setBinary(index++, m_targetIcons[6].GetRawValue());
        stmt->setBinary(index++, m_targetIcons[7].GetRawValue());
        stmt->setUInt8(index++, uint8(m_groupFlags));
        stmt->setUInt32(index++, uint8(m_dungeonDifficulty != DIFFICULTY_MYTHIC_KEYSTONE ? m_dungeonDifficulty : DIFFICULTY_MYTHIC_DUNGEON));
        stmt->setUInt32(index++, uint8(m_legacyRaidDifficulty));
        stmt->setUInt32(index++, uint8(m_raidDifficulty));

        CharacterDatabase.Execute(stmt);


        ASSERT(AddMember(leader)); // If the leader can't be added to a new group because it appears full, something is clearly wrong.

        if (!isLFGGroup())
            Player::ConvertInstancesToGroup(leader, this, false);
    }
    else if (!AddMember(leader))
        return false;

    return true;
}

void Group::LoadGroupFromDB(Field* fields)
{
    m_dbStoreId = fields[15].GetUInt32();
    m_guid = ObjectGuid::Create<HighGuid::Party>(sGroupMgr->GenerateGroupId(), 0);
    m_leaderGuid = ObjectGuid::Create<HighGuid::Player>(fields[0].GetUInt64());

    // group leader not exist
    if (!ObjectMgr::GetPlayerNameByGUID(GetLeaderGUID(), m_leaderName))
        return;

    m_lootMethod = LootMethod(fields[1].GetUInt8());
    m_looterGuid = ObjectGuid::Create<HighGuid::Player>(fields[2].GetUInt64());
    m_lootThreshold = ItemQualities(fields[3].GetUInt8());

    for (uint8 i = 0; i < TARGET_ICONS_COUNT; ++i)
        m_targetIcons[i].SetRawValue(fields[4 + i].GetBinary());

    m_groupFlags = GroupFlags(fields[12].GetUInt8());
    if (m_groupFlags & GROUP_FLAG_RAID)
        _initRaidSubGroupsCounter();

    m_dungeonDifficulty = Player::CheckLoadedDungeonDifficultyID(Difficulty(fields[13].GetUInt8()));
    m_raidDifficulty = Player::CheckLoadedRaidDifficultyID(Difficulty(fields[14].GetUInt8()));
    m_legacyRaidDifficulty = Player::CheckLoadedRaidDifficultyID(Difficulty(fields[15].GetUInt8()));

    if (m_groupFlags & GROUP_FLAG_LFG)
        sLFGMgr->_LoadFromDB(fields, GetGUID());
}

void Group::LoadMemberFromDB(ObjectGuid::LowType guidLow, uint8 memberFlags, uint8 subgroup, uint8 roles)
{
    MemberSlot member;
    member.Guid = ObjectGuid::Create<HighGuid::Player>(guidLow);

    // skip non-existed member
    if (!ObjectMgr::GetPlayerNameByGUID(member.Guid, member.Name))
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GROUP_MEMBER);
        stmt->setUInt64(0, guidLow);
        CharacterDatabase.Execute(stmt);
        return;
    }

    member.Group = subgroup;
    member.Flags = memberFlags;
    member.Roles = roles;

    m_memberSlots.push_back(member);

    SubGroupCounterIncrease(subgroup);

    sLFGMgr->SetupGroupMember(member.Guid, GetGUID());
}

void Group::ChangeFlagEveryoneAssistant(bool apply)
{
    if (apply)
        m_groupFlags = GroupFlags(m_groupFlags | GROUP_FLAG_EVERYONE_ASSISTANT);
    else
        m_groupFlags = GroupFlags(m_groupFlags &~GROUP_FLAG_EVERYONE_ASSISTANT);

    SendUpdate();
}

void Group::ChangeFlagGuildGroup(bool apply)
{
    if (!apply && !IsGuildGroup() || apply && IsGuildGroup())
        return;

    if (apply)
        m_groupFlags = GroupFlags(m_groupFlags | GROUP_FLAG_GUILD_GROUP);
    else
        m_groupFlags = GroupFlags(m_groupFlags &~GROUP_FLAG_GUILD_GROUP);

    SendUpdate();
}

bool Group::IsGuildGroup() const
{
    return (m_groupFlags & GROUP_FLAG_GUILD_GROUP) != 0;
}

void Group::ConvertToLFG(lfg::LFGDungeonData const* dungeon, bool update /*= true*/)
{
    if (dungeon->dbc->IsRaidType())
        ConvertToRaid(!update);

    m_dungeon = dungeon;

    if (update)
    {
        m_groupFlags = GroupFlags(m_groupFlags | GROUP_FLAG_LFG | GROUP_FLAG_LFG_RESTRICTED);
        m_groupCategory = GROUP_CATEGORY_INSTANCE;
        m_lootMethod = PERSONAL_LOOT;

        if (!isBGGroup() && !isBFGroup())
        {
            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GROUP_TYPE);
        
            stmt->setUInt8(0, uint8(m_groupFlags));
            stmt->setUInt32(1, m_dbStoreId);
        
            CharacterDatabase.Execute(stmt);
        }
        SendUpdate();
    }
}

void Group::ConvertToRaid(bool update /*= true*/)
{
    m_groupFlags = GroupFlags(m_groupFlags | GROUP_FLAG_RAID);
    m_lootMethod = PERSONAL_LOOT;

    _initRaidSubGroupsCounter();

    if (update && !isBGGroup() && !isBFGroup())
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GROUP_TYPE);

        stmt->setUInt8(0, uint8(m_groupFlags));
        stmt->setUInt32(1, m_dbStoreId);

        CharacterDatabase.Execute(stmt);
    }

    if (update)
        SendUpdate();

    // update quest related GO states (quest activity dependent from raid membership)
    for (member_citerator citr = m_memberSlots.begin(); citr != m_memberSlots.end(); ++citr)
        if (Player* player = ObjectAccessor::FindPlayer(citr->Guid))
            player->AddDelayedEvent(100, [player]() -> void
        {
            if (player)
            {
                player->UpdateAreaQuestTasks(0, player->GetCurrentAreaID());
                player->UpdateAreaQuestTasks(player->GetCurrentAreaID(), 0);
                player->UpdateForQuestWorldObjects();
            }
        });
}

void Group::ConvertToGroup()
{
    if (m_memberSlots.size() > 5)
        return; // What message error should we send?

    m_groupFlags = GROUP_FLAG_NONE;
    m_lootMethod = PERSONAL_LOOT;

    if (m_subGroupsCounts)
    {
        delete[] m_subGroupsCounts;
        m_subGroupsCounts = nullptr;
    }

    if (!isBGGroup() && !isBFGroup())
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GROUP_TYPE);

        stmt->setUInt8(0, uint8(m_groupFlags));
        stmt->setUInt32(1, m_dbStoreId);

        CharacterDatabase.Execute(stmt);
    }

    SendUpdate();

    // update quest related GO states (quest activity dependent from raid membership)
    for (member_citerator citr = m_memberSlots.begin(); citr != m_memberSlots.end(); ++citr)
        if (Player* player = ObjectAccessor::FindPlayer(citr->Guid))
            player->AddDelayedEvent(100, [player]() -> void
        {
            if (player)
            {
                player->UpdateAreaQuestTasks(0, player->GetCurrentAreaID());
                player->UpdateAreaQuestTasks(player->GetCurrentAreaID(), 0);
                player->UpdateForQuestWorldObjects();
            }
        });
}

bool Group::AddInvite(Player* player)
{
    if (!player || player->GetGroupInvite())
        return false;
    Group* group = player->GetGroup();
    if (group && (group->isBGGroup() || group->isBFGroup()))
        group = player->GetOriginalGroup();
    if (group)
        return false;

    RemoveInvite(player);

    m_invitees.insert(player);

    player->SetGroupInvite(this);

    sScriptMgr->OnGroupInviteMember(this, player->GetGUID());

    return true;
}

bool Group::AddLeaderInvite(Player* player)
{
    if (!AddInvite(player))
        return false;

    m_leaderGuid = player->GetGUID();
    m_leaderName = player->GetName();
    return true;
}

void Group::RemoveInvite(Player* player)
{
    if (player)
    {
        if (!m_invitees.empty())
            m_invitees.erase(PlayerHashGen(player));
        player->SetGroupInvite(nullptr);
    }
}

void Group::RemoveAllInvites()
{
    for (InvitesList::iterator itr = m_invitees.begin(); itr != m_invitees.end(); ++itr)
        if (Player* player = *itr)
            player->SetGroupInvite(nullptr);

    m_invitees.clear();
}

Player* Group::GetInvited(ObjectGuid guid)
{
    for (InvitesList::iterator itr = m_invitees.begin(); itr != m_invitees.end(); ++itr)
    {
        if (Player* player = *itr)
            if (player->GetGUID() == guid)
                return player;
    }
    return nullptr;
}

Player* Group::GetInvited(std::string const& name)
{
    for (InvitesList::iterator itr = m_invitees.begin(); itr != m_invitees.end(); ++itr)
    {
        if (Player* player = *itr)
            if (player->GetName() == name)
                return player;
    }
    return nullptr;
}

bool Group::AddCreatureMember(Creature* creature)
{
    uint8 subGroup = 0;
    if (m_subGroupsCounts)
    {
        bool groupFound = false;
        for (; subGroup < MAX_RAID_SUBGROUPS; ++subGroup)
        {
            if (m_subGroupsCounts[subGroup] < MAX_GROUP_SIZE)
            {
                groupFound = true;
                break;
            }
        }
        if (!groupFound)
            return false;
    }

    MemberSlot member;
    member.Guid = creature->GetGUID();
    member.Name = creature->GetName();
    member.Group = subGroup;
    member.Class = creature->getClass();
    m_memberSlots.push_back(member);

    SubGroupCounterIncrease(subGroup);
    SendUpdate();
    return true;
}

bool Group::AddMember(Player* player)
{
    // Get first not-full group
    uint8 subGroup = 0;
    if (m_subGroupsCounts)
    {
        bool groupFound = false;
        for (; subGroup < MAX_RAID_SUBGROUPS; ++subGroup)
        {
            if (m_subGroupsCounts[subGroup] < MAX_GROUP_SIZE)
            {
                groupFound = true;
                break;
            }
        }
        // We are raid group and no one slot is free
        if (!groupFound)
            return false;
    }

    MemberSlot member;
    if (player)
    {
        member.Guid = player->GetGUID();
        member.Name = player->GetName();
        member.Class = player->getClass();
    }

    member.Group = subGroup;
    m_memberSlots.push_back(member);

    SubGroupCounterIncrease(subGroup);

    if (player)
    {
        player->SetGroupInvite(nullptr);

        bool PvPGroup = isBGGroup() || isBFGroup();

        if (player->GetGroup())
        {
            if (PvPGroup) // if player is in group and he is being added to BG raid group, then call SetBattlegroundRaid()
                player->SetBattlegroundOrBattlefieldRaid(this, subGroup);
            else //if player is in bg raid and we are adding him to normal group, then call SetOriginalGroup()
                player->SetOriginalGroup(this, subGroup);
        }
        else //if player is not in group, then call set group
            player->SetGroup(this, subGroup);

        player->SetPartyType(m_groupCategory, PvPGroup ? GROUP_TYPE_BG : GROUP_TYPE_NORMAL);
        player->ResetGroupUpdateSequenceIfNeeded(this);

        // if the same group invites the player back, cancel the homebind timer
        InstanceGroupBind* bind = GetBoundInstance(player);
        if (bind && bind->save->GetInstanceId() == player->GetInstanceId())
            player->m_InstanceValid = true;
    }

    if (sLFGListMgr->IsGroupQueued(this))
        sLFGListMgr->PlayerAddedToGroup(player, this);

    for (auto& itr : m_targetIcons)
        itr.Clear();

    // insert into the table if we're not a battleground group
    if (!isBGGroup() && !isBFGroup())
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_GROUP_MEMBER);

        stmt->setUInt32(0, m_dbStoreId);
        stmt->setUInt64(1, member.Guid.GetCounter());
        stmt->setUInt8(2, member.Flags);
        stmt->setUInt8(3, member.Group);
        stmt->setUInt8(4, member.Roles);

        CharacterDatabase.Execute(stmt);

    }

    SendUpdate();

    if (player && player->CanContact())
    {
        sScriptMgr->OnGroupAddMember(this, player->GetGUID());

        if (!IsLeader(player->GetGUID()) && !isBGGroup() && !isBFGroup())
        {
            // reset the new member's instances, unless he is currently in one of them
            // including raid/heroic instances that they are not permanently bound to!
            player->ResetInstances(INSTANCE_RESET_GROUP_JOIN, false, false);
            player->ResetInstances(INSTANCE_RESET_GROUP_JOIN, true, false);
            player->ResetInstances(INSTANCE_RESET_GROUP_JOIN, true, true);

            if (player->getLevel() >= LEVELREQUIREMENT_HEROIC)
            {
                if (player->GetDungeonDifficultyID() != GetDungeonDifficultyID())
                {
                    player->SetDungeonDifficultyID(GetDungeonDifficultyID());
                    player->SendDungeonDifficulty();
                }
                if (player->GetRaidDifficultyID() != GetRaidDifficultyID())
                {
                    player->SetRaidDifficultyID(GetRaidDifficultyID());
                    player->SendRaidDifficulty(false);
                }
                if (player->GetLegacyRaidDifficultyID() != GetLegacyRaidDifficultyID())
                {
                    player->SetLegacyRaidDifficultyID(GetLegacyRaidDifficultyID());
                    player->SendRaidDifficulty(true);
                }
            }
        }

        player->SetGroupUpdateFlag(GROUP_UPDATE_FULL);
        if (Pet* pet = player->GetPet())
            pet->SetGroupUpdateFlag(GROUP_UPDATE_PET_FULL);

        UpdatePlayerOutOfRange(player);

        // quest related GO state dependent from raid membership
        if (isRaidGroup())
            player->AddDelayedEvent(100, [player]() -> void
            {
                if (player)
                    player->UpdateForQuestWorldObjects();
            });

        player->SetFieldNotifyFlag(UF_FLAG_PARTY_MEMBER);

        UpdateData groupData(player->GetMapId());
        WorldPacket groupDataPacket;

        // Broadcast group members' fields to player
        for (GroupReference* itr = GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            if (itr->getSource() == player)
                continue;

            if (Player* gMember = itr->getSource())
            {
                if (player->HaveAtClient(gMember))   // must be on the same map, or shit will break
                {
                    gMember->SetFieldNotifyFlag(UF_FLAG_PARTY_MEMBER);
                    gMember->BuildValuesUpdateBlockForPlayer(&groupData, player);
                    gMember->RemoveFieldNotifyFlag(UF_FLAG_PARTY_MEMBER);
                }

                if (gMember->HaveAtClient(player))
                {
                    UpdateData newData(player->GetMapId());
                    WorldPacket newDataPacket;
                    player->BuildValuesUpdateBlockForPlayer(&newData, gMember);
                    if (newData.HasData())
                        if (newData.BuildPacket(&newDataPacket))
                            gMember->SendDirectMessage(&newDataPacket);
                }
            }
        }

        if (groupData.HasData())
            if (groupData.BuildPacket(&groupDataPacket))
                player->SendDirectMessage(&groupDataPacket);

        player->RemoveFieldNotifyFlag(UF_FLAG_PARTY_MEMBER);
    }

    if (m_maxEnchantingLevel < player->GetSkillValue(SKILL_ENCHANTING))
        m_maxEnchantingLevel = player->GetSkillValue(SKILL_ENCHANTING);

    if (auto leader = ObjectAccessor::FindPlayer(GetLeaderGUID()))
    {
        if (auto guidl = sGuildMgr->GetGuildById(leader->GetGuildId()))
            ChangeFlagGuildGroup(IsGuildGroup(guidl->GetGUID()));
        else
            ChangeFlagGuildGroup(false);
    }

    return true;
}

bool Group::AddMysteryMember(ObjectGuid::LowType guidLow, std::string Name, uint8 Class, uint8 Roles, uint8 Flags, bool fakeOnline)
{
    MemberSlot member;
    member.Guid = ObjectGuid::Create<HighGuid::Player>(guidLow);

    // skip non-existed member
    if (!ObjectMgr::GetPlayerNameByGUID(member.Guid, member.Name))
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GROUP_MEMBER);
        stmt->setUInt64(0, guidLow);
        CharacterDatabase.Execute(stmt);
        return false;
    }

    uint8 subGroup = 0;
    if (m_subGroupsCounts)
    {
        bool groupFound = false;
        for (; subGroup < MAX_RAID_SUBGROUPS; ++subGroup)
        {
            if (m_subGroupsCounts[subGroup] < MAX_GROUP_SIZE)
            {
                groupFound = true;
                break;
            }
        }
        if (!groupFound)
            return false;
    }

    member.Name = std::move(Name);
    member.Class = Class;
    member.Roles = Roles;
    member.fakeOnline = fakeOnline;
    member.Group = subGroup;
    member.Flags = Flags;

    m_memberSlots.push_back(member);

    SubGroupCounterIncrease(subGroup);
    //SendUpdate();
    return true;
}

bool Group::RemoveCreatureMember(ObjectGuid const& guid)
{
    if (guid.IsEmpty())
        return false;

    BroadcastGroupUpdate();

    auto slot = _getMemberWSlot(guid);
    if (slot != m_memberSlots.end())
    {
        SubGroupCounterDecrease(slot->Group);
        m_memberSlots.erase(slot);
    }

    SendUpdate();

    return true;
}

bool Group::RemoveMember(ObjectGuid const& guid, bool /*disbandInfo*/, RemoveMethod const& method /*= GROUP_REMOVEMETHOD_DEFAULT*/, ObjectGuid kicker /*= 0*/, char const* reason /*= NULL*/)
{
    // if (disbandInfo)
        return RemoveMemberQueue(guid, method, kicker, reason);
    // else
    // {
        // m_Functions.AddFunction([this, guid, method, kicker, reason]() -> void
        // {
            // if (!this)
                // return;
            // RemoveMemberQueue(guid, method, kicker, reason);
        // }, m_Functions.CalculateTime(10));
    // }

    //return false;
}

bool Group::RemoveMemberQueue(ObjectGuid const& guid, RemoveMethod const& method /*= GROUP_REMOVEMETHOD_DEFAULT*/, ObjectGuid kicker /*= 0*/, char const* reason /*= NULL*/)
{
    // std::lock_guard<std::recursive_mutex> _lock(m_lock);
    BroadcastGroupUpdate();

    sScriptMgr->OnGroupRemoveMember(this, guid, method, kicker, reason);

    if (Player* player = sObjectAccessor->FindPlayer(guid))
        if (sLFGListMgr->IsGroupQueued(this))
            sLFGListMgr->PlayerRemoveFromGroup(player, this);

    // LFG group vote kick handled in scripts
    if (isLFGGroup() && method == GROUP_REMOVEMETHOD_KICK)
        return !m_memberSlots.empty();

    // remove member and change leader (if need) only if strong more 2 members _before_ member remove (BG/BF allow 1 member group)
    if (sLFGListMgr->IsGroupQueued(this) || GetMembersCount() > (isBGGroup() || isLFGGroup() || isBFGroup() ? 1u : 2u))
    {
        Player* player = ObjectAccessor::GetObjectInOrOutOfWorld(guid, static_cast<Player*>(nullptr));
        if (player)
        {
            if (isBGGroup() || isBFGroup()) // Battleground group handling
                player->RemoveFromBattlegroundOrBattlefieldRaid();
            else // Regular group
            {
                if (player->GetOriginalGroup() == this)
                    player->SetOriginalGroup(nullptr);
                else
                    player->SetGroup(nullptr);

                // quest related GO state dependent from raid membership
                player->AddDelayedEvent(100, [player]() -> void
                {
                    if (player)
                        player->UpdateForQuestWorldObjects();
                });
            }

            player->SetPartyType(m_groupCategory, GROUP_TYPE_NONE);

            if (method == GROUP_REMOVEMETHOD_KICK || method == GROUP_REMOVEMETHOD_KICK_LFG)
                player->SendDirectMessage(WorldPackets::Party::GroupUninvite().Write());

            WorldPackets::Party::PartyUpdate update;
            update.PartyFlags = m_groupFlags;
            update.PartyIndex = m_groupCategory;
            update.PartyType = IsCreated() ? GROUP_TYPE_NORMAL : GROUP_TYPE_NONE;
            update.MyIndex = -1;
            update.PartyGUID = GetGUID();
            update.SequenceNum = player->NextGroupUpdateSequenceNumber(m_groupCategory);
            player->SendDirectMessage(update.Write());

            if (!isLFGGroup())
                _homebindIfInstance(player);
        }

        // Remove player from group in DB
        if (!isBGGroup() && !isBFGroup())
        {
            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GROUP_MEMBER);
            stmt->setUInt64(0, guid.GetCounter());
            CharacterDatabase.Execute(stmt);
            DelinkMember(guid);
        }

        // Reevaluate group enchanter if the leaving player had enchanting skill or the player is offline
        if (player && player->GetSkillValue(SKILL_ENCHANTING) || !player)
            ResetMaxEnchantingLevel();

        // Remove player from loot rolls
        for (auto itr : RollId)
        {
            auto itr2 = itr->playerVote.find(guid);
            if (itr2 == itr->playerVote.end())
                continue;

            if (itr2->second == GREED || itr2->second == DISENCHANT)
                --itr->totalGreed;
            else if (itr2->second == NEED)
                --itr->totalNeed;
            else if (itr2->second == PASS)
                --itr->totalPass;

            if (itr2->second != NOT_VALID)
                --itr->totalPlayersRolling;

            itr->playerVote.erase(itr2);

            CountRollVote(guid, itr->aoeSlot, MAX_ROLL_TYPE);
        }

        // Update subgroups
        auto slot = _getMemberWSlot(guid);
        if (slot != m_memberSlots.end())
        {
            std::lock_guard<std::recursive_mutex> _lock(m_lock);
            SubGroupCounterDecrease(slot->Group);
            m_memberSlots.erase(slot);
        }

        // Pick new leader if necessary
        if (GetLeaderGUID() == guid)
        {
            for (auto& itr : m_memberSlots)
            {
                if (ObjectAccessor::FindPlayer(itr.Guid))
                {
                    ChangeLeader(itr.Guid);
                    break;
                }
            }
        }

        if (auto leader = ObjectAccessor::FindPlayer(GetLeaderGUID()))
        {
            if (auto guidl = sGuildMgr->GetGuildById(leader->GetGuildId()))
                ChangeFlagGuildGroup(IsGuildGroup(guidl->GetGUID()));
            else
                ChangeFlagGuildGroup(false);
        }

        SendUpdate();

        if (isLFGGroup() && GetMembersCount() == 1)
        {
            Player* leader = ObjectAccessor::FindPlayer(GetLeaderGUID());
            uint32 mapId = sLFGMgr->GetDungeonMapId(GetGUID());
            if (!mapId || !leader || leader->isAlive() && leader->GetMapId() != mapId)
            {
                Disband();
                return false;
            }
        }

        if ((!sLFGListMgr->IsGroupQueued(this) || !m_memberMgr.getSize()) && m_memberMgr.getSize() < (isLFGGroup() || isBGGroup() ? 1u : 2u))
            Disband();
        else if (player)
            SendUpdateDestroyGroupToPlayer(player);

        return true;
    }

    Disband();
    return false;
}

void Group::ChangeLeader(ObjectGuid const& guid, int8 partyIndex /*= 0*/)
{
    auto slot = _getMemberWSlot(guid);
    if (slot == m_memberSlots.end())
        return;

    Player* player = ObjectAccessor::FindPlayer(slot->Guid);
    if (!player)
        return;

    sScriptMgr->OnGroupChangeLeader(this, GetLeaderGUID(), guid);

    if (!isBGGroup() && !isBFGroup())
    {
        std::lock_guard<std::recursive_mutex> _lock(m_bound_lock);
        // Remove the groups permanent instance bindings
        for (auto& itr : m_boundInstances)
        {
            for (auto itr2 = itr.begin(); itr2 != itr.end();)
            {
                if (itr2->second.perm)
                {
                    itr2->second.save->RemoveGroup(this);
                    itr.erase(itr2++);
                }
                else
                    ++itr2;
            }
        }

        // Same in the database

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GROUP_INSTANCE_PERM_BINDING);

        stmt->setUInt32(0, m_dbStoreId);
        stmt->setUInt64(1, player->GetGUIDLow());

        CharacterDatabase.Execute(stmt);

        // Copy the permanent binds from the new leader to the group
        if (!isLFGGroup())
            Player::ConvertInstancesToGroup(player, this, true);

        // Update the group leader
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GROUP_LEADER);

        stmt->setUInt64(0, player->GetGUIDLow());
        stmt->setUInt32(1, m_dbStoreId);

        CharacterDatabase.Execute(stmt);
    }

    if (Player* oldLeader = ObjectAccessor::FindPlayer(GetLeaderGUID()))
        oldLeader->RemoveFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_GROUP_LEADER);

    if (slot == m_memberSlots.end())
        return;

    player->SetFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_GROUP_LEADER);
    m_leaderGuid = player->GetGUID();
    m_leaderName = player->GetName();
    ToggleGroupMemberFlag(slot, MEMBER_FLAG_ASSISTANT, false);

    WorldPackets::Party::GroupNewLeader groupNewLeader;
    groupNewLeader.Name = slot->Name;
    groupNewLeader.PartyIndex = partyIndex;
    BroadcastPacket(groupNewLeader.Write(), true);
}

void Group::ChangeLeaderOffline(ObjectGuid const& guid, std::string Name)
{
    m_leaderGuid = guid;
    m_leaderName = std::move(Name);
}

void Group::Disband(bool hideDestroy /* = false */)
{
    sScriptMgr->OnGroupDisband(this);
    sLFGListMgr->Remove(GetGUIDLow(), nullptr, false);

    for (member_citerator citr = m_memberSlots.begin(); citr != m_memberSlots.end(); ++citr)
    {
        Player* player = ObjectAccessor::FindPlayer(citr->Guid);
        if (!player || !player->CanContact())
            continue;

        //we cannot call _removeMember because it would invalidate member iterator
        //if we are removing player from battleground raid
        if (isBGGroup() || isBFGroup())
            player->RemoveFromBattlegroundOrBattlefieldRaid();
        else
        {
            //we can remove player who is in battleground from his original group
            if (player->GetOriginalGroup() == this)
                player->SetOriginalGroup(nullptr);
            else
                player->SetGroup(nullptr);
        }

        player->SetPartyType(m_groupCategory, GROUP_TYPE_NONE);

        // quest related GO state dependent from raid membership
        if (isRaidGroup())
            player->AddDelayedEvent(100, [player]() -> void
        {
            if (player)
                player->UpdateForQuestWorldObjects();
        });

        if (!hideDestroy)
            player->SendDirectMessage(WorldPackets::Party::GroupDestroyed().Write());

        SendUpdateDestroyGroupToPlayer(player);

        if (!isLFGGroup())
            _homebindIfInstance(player);
    }
    RollId.clear();
    m_memberSlots.clear();

    RemoveAllInvites();

    if (!isBGGroup() && !isBFGroup())
    {
        SQLTransaction trans = CharacterDatabase.BeginTransaction();

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GROUP);
        stmt->setUInt32(0, m_dbStoreId);
        trans->Append(stmt);

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GROUP_MEMBER_ALL);
        stmt->setUInt32(0, m_dbStoreId);
        trans->Append(stmt);

        CharacterDatabase.CommitTransaction(trans);

        ResetInstances(INSTANCE_RESET_GROUP_DISBAND, false, false, nullptr);
        ResetInstances(INSTANCE_RESET_GROUP_DISBAND, true, false, nullptr);
        ResetInstances(INSTANCE_RESET_GROUP_DISBAND, true, true, nullptr);

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_LFG_DATA);
        stmt->setUInt32(0, m_dbStoreId);
        CharacterDatabase.Execute(stmt);

        sGroupMgr->FreeGroupDbStoreId(this);
    }

    sGroupMgr->RemoveGroup(this);
    delete this;
}

/*********************************************************/
/***                   LOOT SYSTEM                     ***/
/*********************************************************/

void Group::SendLootStartRoll(uint32 mapID, Roll const& roll)
{
    WorldPackets::Loot::StartLootRoll lootRoll;
    roll.FillPacket(lootRoll.Item);
    lootRoll.LootObj = roll.lootedGUID;
    lootRoll.MapID = mapID;
    lootRoll.Item.UIType = LOOT_ITEM_UI_ROLL; // or LOOT_ITEM_UI_ROLL. same from SMSG_LOOT_RESPONSE but no metter that correct
    lootRoll.RollTime = isRaidGroup() ? RAID_ROLL_TIMER : NORMAL_ROLL_TIMER;
    lootRoll.ValidRolls = roll.totalPlayersRolling;
    lootRoll.Method = GetLootMethod();

    WorldPacket const* p = lootRoll.Write();
    for (const auto& itr : roll.playerVote)
    {
        Player* player = ObjectAccessor::FindPlayer(itr.first);
        if (!player || !player->CanContact())
            continue;

        if (itr.second == NOT_EMITED_YET)
            player->SendDirectMessage(p);
    }
}

void Group::SendLootStartRollToPlayer(uint32 mapID, Player* player, bool canNeed, Roll const& roll)
{
    if (!player || !player->CanContact())
        return;

    WorldPackets::Loot::StartLootRoll lootRoll;
    roll.FillPacket(lootRoll.Item);
    lootRoll.LootObj = roll.lootedGUID;
    lootRoll.MapID = mapID;
    lootRoll.Item.UIType = LOOT_ITEM_UI_ROLL;
    lootRoll.RollTime = isRaidGroup() ? RAID_ROLL_TIMER : NORMAL_ROLL_TIMER;
    lootRoll.ValidRolls = roll.rollVoteMask;
    if (!canNeed)
        lootRoll.ValidRolls &= ~ROLL_FLAG_TYPE_NEED;
    lootRoll.Method = GetLootMethod();
    player->SendDirectMessage(lootRoll.Write());
}

void Group::SendLootRoll(ObjectGuid targetGuid, uint8 rollNumber, uint8 rollType, Roll const& roll)
{
    WorldPackets::Loot::LootRollResponse response;
    roll.FillPacket(response.LootItems);
    response.LootItems.UIType = 2;
    response.LootObj = roll.lootedGUID;
    response.Player = targetGuid;
    response.Roll = rollNumber;
    response.RollType = rollType;
    response.Autopassed = false;

    WorldPacket const* pdata = response.Write();
    for (const auto& itr : roll.playerVote)
    {
        Player* p = ObjectAccessor::FindPlayer(itr.first);
        if (!p || !p->CanContact())
            continue;

        if (itr.second != NOT_VALID)
            p->SendDirectMessage(pdata);
    }
}

void Group::SendLootRollWon(ObjectGuid targetGuid, uint8 rollNumber, uint8 rollType, Roll const& roll)
{
    WorldPackets::Loot::LootRollWon won;
    won.LootObj = roll.lootedGUID;
    won.Player = targetGuid;
    won.Roll = rollNumber;
    won.RollType = rollType;
    won.LootItems.UIType = 1;
    roll.FillPacket(won.LootItems);

    // roll.TotalEmited()

    WorldPackets::Loot::LootRollsComplete rollsComplete;
    rollsComplete.LootListID = roll.itemSlot;
    rollsComplete.LootObj = roll.lootedGUID;

    WorldPacket const* pwon = won.Write();
    WorldPacket const* prollsComplete = rollsComplete.Write();

    for (const auto& itr : roll.playerVote)
    {
        Player* p = ObjectAccessor::FindPlayer(itr.first);
        if (!p || !p->CanContact())
            continue;

        if (itr.second != NOT_VALID)
        {
            p->SendDirectMessage(pwon);
            p->SendDirectMessage(prollsComplete);
        }
    }
}

void Group::SendLootAllPassed(Roll const& roll)
{
    WorldPackets::Loot::LootAllPassed  passed;
    roll.FillPacket(passed.LootItems);
    passed.LootObj = roll.lootedGUID;
    passed.LootItems.UIType = 1;
    // roll.TotalEmited() ???

    WorldPacket const* ppassed = passed.Write();

    for (const auto& itr : roll.playerVote)
    {
        Player* player = ObjectAccessor::FindPlayer(itr.first);
        if (!player || !player->CanContact())
            continue;

        if (itr.second != NOT_VALID)
            player->SendDirectMessage(ppassed);
    }
}

void Group::SendLooter(Creature* creature, Player* groupLooter)
{
    ASSERT(creature);

    WorldPackets::Loot::LootList lootList;
    lootList.Owner = creature->GetGUID();
    lootList.LootObj = creature->loot.GetGUID();

    if (GetLootMethod() == MASTER_LOOT && creature->loot.hasOverThresholdItem())
        lootList.Master = m_looterGuid;

    if (groupLooter)
        lootList.RoundRobinWinner = groupLooter->GetGUID();

    BroadcastPacket(lootList.Write(), false);
}

void Group::GroupLoot(Loot* loot, WorldObject* pLootedObject)
{
    std::vector<LootItem>::iterator i;
    ItemTemplate const* item;
    uint8 itemSlot = 0;

    for (i = loot->items.begin(); i != loot->items.end(); ++i, ++itemSlot)
    {
        if (i->freeforall || i->currency)
            continue;

        item = sObjectMgr->GetItemTemplate(i->item.ItemID);
        if (!item)
        {
            //TC_LOG_DEBUG(LOG_FILTER_GENERAL, "Group::GroupLoot: missing item prototype for item with id: %d", i->itemid);
            continue;
        }

        //roll for over-threshold item if it's one-player loot
        if (item->GetQuality() >= uint32(m_lootThreshold))
        {
            ObjectGuid newitemGUID = ObjectGuid::Create<HighGuid::Item>(sObjectMgr->GetGenerator<HighGuid::Item>()->Generate());
            auto r = new Roll(newitemGUID, *i);
            r->lootedGUID = loot->GetGUID();

            //a vector is filled with only near party members
            for (GroupReference* itr = GetFirstMember(); itr != nullptr; itr = itr->next())
            {
                Player* member = itr->getSource();
                if (!member || !member->CanContact())
                    continue;
                if (loot->AllowedForPlayer(member, i->item.ItemID, i->type, i->needs_quest, &*i))
                {
                    if (member->IsWithinDistInMap(pLootedObject, sWorld->getFloatConfig(CONFIG_GROUP_XP_DISTANCE), false))
                    {
                        r->totalPlayersRolling++;

                        if (member->GetPassOnGroupLoot())
                        {
                            r->playerVote[member->GetGUID()] = PASS;
                            r->totalPass++;
                            // can't broadcast the pass now. need to wait until all rolling players are known.
                        }
                        else
                            r->playerVote[member->GetGUID()] = NOT_EMITED_YET;
                    }
                }
            }

            if (r->totalPlayersRolling > 0)
            {
                r->itemSlot = itemSlot;
                r->aoeSlot = ++m_aoe_slots;
                // if (item->DisenchantID && m_maxEnchantingLevel >= item->RequiredDisenchantSkill)
                    // r->rollVoteMask |= ROLL_FLAG_TYPE_DISENCHANT;

                if (item->GetFlags2() & ITEM_FLAG2_CAN_ONLY_ROLL_GREED)
                    r->rollVoteMask &= ~ROLL_FLAG_TYPE_NEED;

                loot->items[itemSlot].is_blocked = true;

                // If there is any "auto pass", broadcast the pass now.
                if (r->totalPass)
                {
                    for (Roll::PlayerVote::const_iterator itr = r->playerVote.begin(); itr != r->playerVote.end(); ++itr)
                    {
                        Player* p = ObjectAccessor::FindPlayer(itr->first);
                        if (!p || !p->CanContact())
                            continue;

                        if (itr->second == PASS)
                            SendLootRoll(p->GetGUID(), 128, ROLL_PASS, *r);
                    }
                }

                SendLootStartRoll(pLootedObject->GetMapId(), *r);

                RollId.push_back(r);

                if (Creature* creature = pLootedObject->ToCreature())
                {
                    creature->m_groupLootTimer = isRaidGroup() ? RAID_ROLL_TIMER : NORMAL_ROLL_TIMER;
                    creature->lootingGroupLowGUID = GetGUID();
                }
                else if (GameObject* go = pLootedObject->ToGameObject())
                {
                    go->m_groupLootTimer = isRaidGroup() ? RAID_ROLL_TIMER : NORMAL_ROLL_TIMER;
                    go->lootingGroupLowGUID = GetGUID();
                }
            }
            else
                delete r;
        }
        else
            i->is_underthreshold = true;
    }

    for (i = loot->quest_items.begin(); i != loot->quest_items.end(); ++i, ++itemSlot)
    {
        if (!i->follow_loot_rules)
            continue;

        item = sObjectMgr->GetItemTemplate(i->item.ItemID);
        if (!item)
            continue;

        ObjectGuid newitemGUID = ObjectGuid::Create<HighGuid::Item>(sObjectMgr->GetGenerator<HighGuid::Item>()->Generate());
        auto r = new Roll(newitemGUID, *i);
        r->lootedGUID = loot->GetGUID();

        //a vector is filled with only near party members
        for (GroupReference* itr = GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->getSource();
            if (!member || !member->CanContact())
                continue;

            if (loot->AllowedForPlayer(member, i->item.ItemID, i->type, i->needs_quest, &*i))
            {
                if (member->IsWithinDistInMap(pLootedObject, sWorld->getFloatConfig(CONFIG_GROUP_XP_DISTANCE), false))
                {
                    r->totalPlayersRolling++;
                    r->playerVote[member->GetGUID()] = NOT_EMITED_YET;
                }
            }
        }

        if (r->totalPlayersRolling > 0)
        {
            r->itemSlot = itemSlot;
            r->aoeSlot = ++m_aoe_slots;

            loot->quest_items[itemSlot - loot->items.size()].is_blocked = true;

            SendLootStartRoll(pLootedObject->GetMapId(), *r);

            RollId.push_back(r);

            if (Creature* creature = pLootedObject->ToCreature())
            {
                creature->m_groupLootTimer = isRaidGroup() ? RAID_ROLL_TIMER : NORMAL_ROLL_TIMER;
                creature->lootingGroupLowGUID = GetGUID();
            }
            else if (GameObject* go = pLootedObject->ToGameObject())
            {
                go->m_groupLootTimer = isRaidGroup() ? RAID_ROLL_TIMER : NORMAL_ROLL_TIMER;
                go->lootingGroupLowGUID = GetGUID();
            }
        }
        else
            delete r;
    }
}

void Group::MasterLoot(Loot* loot, WorldObject* lootObj)
{
    WorldPackets::Loot::MasterLootCandidateList list;
    list.LootObj = loot->GetGUID();
    for (GroupReference* itr = GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* looter = itr->getSource();
        if (!looter->IsInWorld())
            continue;

        if (looter->IsWithinDistInMap(lootObj, sWorld->getFloatConfig(CONFIG_GROUP_XP_DISTANCE), false))
            list.Players.push_back(ObjectGuid::Create<HighGuid::Player>(looter->GetGUIDLow())); //HardHack! Plr should have off-like hiGuid
    }

    if (Player* player = ObjectAccessor::FindPlayer(GetLooterGuid()))
        player->SendDirectMessage(list.Write());
}

void Group::DoRollForAllMembers(ObjectGuid guid, uint8 slot, uint32 mapid, Loot* loot, LootItem& item, Player* player)
{
    // Already rolled?
    for (auto& iter : RollId)
        if (iter->itemSlot == slot && loot == iter->getLoot() && iter->isValid())
            return;

    ObjectGuid newitemGUID = ObjectGuid::Create<HighGuid::Item>(sObjectMgr->GetGenerator<HighGuid::Item>()->Generate());
    auto r = new Roll(newitemGUID, item);
    r->lootedGUID = loot->GetGUID();
    WorldObject* pLootedObject = nullptr;

    if (guid.IsLoot())
        guid = loot->objGuid;

    if (guid.IsCreatureOrVehicle())
        pLootedObject = player->GetMap()->GetCreature(guid);
    else if (guid.IsGameObject())
        pLootedObject = player->GetMap()->GetGameObject(guid);

    if (!pLootedObject)
        return;

    //a vector is filled with only near party members
    for (GroupReference* itr = GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->getSource();
        if (!member || !member->CanContact())
            continue;

        if (loot->AllowedForPlayer(member, item.item.ItemID, item.type, item.needs_quest, &item))
        {
            if (member->IsWithinDistInMap(pLootedObject, sWorld->getFloatConfig(CONFIG_GROUP_XP_DISTANCE), false))
            {
                r->totalPlayersRolling++;
                r->playerVote[member->GetGUID()] = NOT_EMITED_YET;
            }
        }
    }

    if (r->totalPlayersRolling > 0)
    {
        r->itemSlot = slot;
        r->aoeSlot = ++m_aoe_slots;     //restart at next loot. it's normall

        RollId.push_back(r);
    }

    SendLootStartRoll(mapid, *r);
}

void Group::CountRollVote(ObjectGuid playerGUID, uint8 AoeSlot, uint8 Choice)
{
    auto rollI = GetRoll(AoeSlot);
    if (rollI == RollId.end())
        return;

    auto roll = *rollI;

    auto itr = roll->playerVote.find(playerGUID);
    // this condition means that player joins to the party after roll begins
    if (itr == roll->playerVote.end())
        return;

    if (roll->getLoot())
    {
        if (roll->getLoot()->items.empty())
            return;
    }
    else
        return;

    switch (Choice)
    {
        case ROLL_PASS:                                     // Player choose pass
            ++roll->totalPass;
            SendLootRoll(playerGUID, 0, ROLL_PASS, *roll);
            itr->second = PASS;
            break;
        case ROLL_NEED:                                     // player choose Need
            ++roll->totalNeed;
            SendLootRoll(playerGUID, 0, ROLL_NEED, *roll);
            itr->second = NEED;
            break;
        case ROLL_GREED:                                    // player choose Greed
            ++roll->totalGreed;
            SendLootRoll(playerGUID, 0, ROLL_GREED, *roll);
            itr->second = GREED;
            break;
        case ROLL_DISENCHANT:                               // player choose Disenchant
            ++roll->totalGreed;
            SendLootRoll(playerGUID, 0, ROLL_DISENCHANT, *roll);
            itr->second = DISENCHANT;
            break;
        default:
            break;
    }

    if (roll->TotalEmited() >= roll->totalPlayersRolling)
        CountTheRoll(rollI);
}

//called when roll timer expires
void Group::EndRoll(Loot* pLoot)
{
    for (auto itr = RollId.begin(); itr != RollId.end();)
    {
        if ((*itr)->getLoot() == pLoot)
        {
            CountTheRoll(itr);           //i don't have to edit player votes, who didn't vote ... he will pass
            itr = RollId.begin();
        }
        else
            ++itr;
    }
}

void Group::ClearAoeSlots()
{
    m_aoe_slots = 0;
}

bool Group::isRolledSlot(uint8 _slot)
{
    for (auto& iter : RollId)
        if (iter->aoeSlot == _slot && iter->isValid())
            return true;
    return false;
}

bool Group::RollIsActive()
{
    return !RollId.empty();
}

void Group::CountTheRoll(Rolls::iterator rollI)
{
    Roll* roll = *rollI;
    if (!roll->isValid())                                   // is loot already deleted ?
    {
        RollId.erase(rollI);
        delete roll;
        return;
    }

    Loot* loot = roll->getLoot();
    if (!loot)                                              // is loot already deleted ?
    {
        RollId.erase(rollI);
        delete roll;
        return;
    }

    //end of the roll
    if (roll->totalNeed > 0)
    {
        if (!roll->playerVote.empty())
        {
            uint8 maxresul = 0;
            ObjectGuid maxguid = (*roll->playerVote.begin()).first;

            for (Roll::PlayerVote::const_iterator itr = roll->playerVote.begin(); itr != roll->playerVote.end(); ++itr)
            {
                if (itr->second != NEED)
                    continue;

                uint8 randomN = urand(1, 100);
                SendLootRoll(itr->first, randomN, ROLL_NEED, *roll);
                if (maxresul < randomN)
                {
                    maxguid = itr->first;
                    maxresul = randomN;
                }
            }
            SendLootRollWon(maxguid, maxresul, ROLL_NEED, *roll);
            Player * player = ObjectAccessor::FindPlayer(maxguid);

            if (player && player->CanContact())
            {
                player->UpdateAchievementCriteria(CRITERIA_TYPE_ROLL_NEED_ON_LOOT, roll->item.ItemID, maxresul);

                ItemPosCountVec dest;
                LootItem* item = &(roll->itemSlot >= loot->items.size() ? loot->quest_items[roll->itemSlot - loot->items.size()] : loot->items[roll->itemSlot]);
                InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, roll->item.ItemID, item->count);
                if (msg == EQUIP_ERR_OK)
                {
                    item->is_looted = true;
                    loot->NotifyItemRemoved(roll->itemSlot);
                    loot->unlootedCount--;
                    player->StoreNewItem(dest, roll->item.ItemID, true, item->item.RandomPropertiesID, item->GetAllowedLooters(), item->item.ItemBonus.BonusListIDs, item->item.ItemBonus.Context);
                }
                else
                {
                    item->is_blocked = false;
                    player->SendEquipError(msg, nullptr, nullptr, roll->item.ItemID);
                }
            }
        }
    }
    else if (roll->totalGreed > 0)
    {
        if (!roll->playerVote.empty())
        {
            uint8 maxresul = 0;
            ObjectGuid maxguid = (*roll->playerVote.begin()).first;
            RollVote rollvote = NOT_VALID;

            for (auto itr = roll->playerVote.begin(); itr != roll->playerVote.end(); ++itr)
            {
                if (itr->second != GREED && itr->second != DISENCHANT)
                    continue;

                uint8 randomN = urand(1, 100);
                SendLootRoll(itr->first, randomN, itr->second, *roll);
                if (maxresul < randomN)
                {
                    maxguid = itr->first;
                    maxresul = randomN;
                    rollvote = itr->second;
                }
            }
            SendLootRollWon(maxguid, maxresul, rollvote, *roll);
            Player * player = ObjectAccessor::FindPlayer(maxguid);

            if (player && player->CanContact())
            {
                player->UpdateAchievementCriteria(CRITERIA_TYPE_ROLL_GREED_ON_LOOT, roll->item.ItemID, maxresul);

                LootItem* item = &(roll->itemSlot >= loot->items.size() ? loot->quest_items[roll->itemSlot - loot->items.size()] : loot->items[roll->itemSlot]);

                if (rollvote == GREED)
                {
                    ItemPosCountVec dest;
                    InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, roll->item.ItemID, item->count);
                    if (msg == EQUIP_ERR_OK)
                    {
                        item->is_looted = true;
                        loot->NotifyItemRemoved(roll->itemSlot);
                        loot->unlootedCount--;
                        player->StoreNewItem(dest, roll->item.ItemID, true, item->item.RandomPropertiesID, item->GetAllowedLooters(), item->item.ItemBonus.BonusListIDs, item->item.ItemBonus.Context);
                    }
                    else
                    {
                        item->is_blocked = false;
                        player->SendEquipError(msg, nullptr, nullptr, roll->item.ItemID);
                    }
                }
                else if (rollvote == DISENCHANT)
                {
                    item->is_looted = true;
                    loot->NotifyItemRemoved(roll->itemSlot);
                    loot->unlootedCount--;
                    ItemTemplate const* pProto = sObjectMgr->GetItemTemplate(roll->item.ItemID);
                    player->AutoStoreLoot(pProto->GetId(), LootTemplates_Disenchant, false, true);
                    player->UpdateAchievementCriteria(CRITERIA_TYPE_CAST_SPELL, 13262); // Disenchant
                }
            }
        }
    }
    else
    {
        SendLootAllPassed(*roll);

        // remove is_blocked so that the item is lootable by all players
        LootItem* item = &(roll->itemSlot >= loot->items.size() ? loot->quest_items[roll->itemSlot - loot->items.size()] : loot->items[roll->itemSlot]);
        if (item)
            item->is_blocked = false;
    }

    RollId.erase(rollI);
    delete roll;
}

void Group::SetTargetIcon(uint8 symbol, ObjectGuid target, ObjectGuid changedBy, uint8 partyIndex)
{
    if (symbol >= TARGET_ICONS_COUNT)
        return;

    if (!target.IsEmpty())
        for (uint8 i = 0; i < TARGET_ICONS_COUNT; ++i)
            if (m_targetIcons[i] == target)
                SetTargetIcon(i, ObjectGuid::Empty, changedBy, partyIndex);

    m_targetIcons[symbol] = target;

    WorldPackets::Party::SendRaidTargetUpdateSingle updateSingle;
    updateSingle.PartyIndex = partyIndex;
    updateSingle.Target = target;
    updateSingle.ChangedBy = changedBy;
    updateSingle.Symbol = symbol;
    BroadcastPacket(updateSingle.Write(), true);
}

void Group::SendTargetIconList(int8 partyIndex)
{
    WorldPackets::Party::SendRaidTargetUpdateAll updateAll;
    updateAll.PartyIndex = partyIndex;
    for (uint8 i = 0; i < TARGET_ICONS_COUNT; i++)
        updateAll.TargetIcons.insert(std::make_pair(i, m_targetIcons[i]));

    BroadcastPacket(updateAll.Write(), true);
}

void Group::SendUpdate()
{
    for (auto& itr : m_memberSlots)
        SendUpdateToPlayer(itr.Guid, &itr);
}

void Group::SendUpdateToPlayer(ObjectGuid playerGUID, MemberSlot* slot /*= nullptr*/)
{
    if (!playerGUID.IsPlayer())
        return;

    Player* player = ObjectAccessor::FindPlayer(playerGUID);
    if (!player || !player->GetSession()/*!player->CanContact()*/ || player->GetGroup() != this)
        return;

    // if MemberSlot wasn't provided
    if (!slot)
    {
        auto witr = _getMemberWSlot(playerGUID);
        if (witr == m_memberSlots.end()) // if there is no MemberSlot for such a player
            return;

        slot = &*witr;
    }

    WorldPackets::Party::PartyUpdate partyUpdate;
    bool PvPGroup = isBGGroup() || isBFGroup();
    partyUpdate.PartyType = PvPGroup ? GROUP_TYPE_BG : GROUP_TYPE_NORMAL;
    partyUpdate.PartyFlags = m_groupFlags;
    partyUpdate.PartyGUID = m_guid;
    partyUpdate.LeaderGUID = GetLeaderGUID();
    partyUpdate.SequenceNum = player->NextGroupUpdateSequenceNumber(m_groupCategory);
    partyUpdate.PartyIndex = m_groupCategory;
    partyUpdate.MyIndex = -1;

    uint8 index = 0;
    for (member_citerator citr = m_memberSlots.begin(); citr != m_memberSlots.end(); ++citr, ++index)
    {
        if (slot->Guid == citr->Guid)
            partyUpdate.MyIndex = index;

        Player* member = ObjectAccessor::FindPlayer(citr->Guid);

        WorldPackets::Party::GroupPlayerInfos playerInfos;

        playerInfos.GUID = citr->Guid;
        playerInfos.Name = citr->Name;
        playerInfos.Class = citr->Class;

        playerInfos.Status = citr->fakeOnline ? MEMBER_STATUS_ONLINE : MEMBER_STATUS_OFFLINE;
        if (member && member->CanContact())
            playerInfos.Status = MEMBER_STATUS_ONLINE | (isBGGroup() || isBFGroup() ? MEMBER_STATUS_PVP : 0);

        playerInfos.Subgroup = citr->Group;
        playerInfos.Flags = citr->Flags;
        playerInfos.RolesAssigned = citr->Roles;
        playerInfos.FromSocialQueue = false;

        partyUpdate.PlayerList.push_back(playerInfos);
    }

    if (GetMembersCount())
    {
        partyUpdate.LootSettings = boost::in_place();
        partyUpdate.LootSettings->Method = m_lootMethod;
        partyUpdate.LootSettings->Threshold = m_lootThreshold;
        partyUpdate.LootSettings->LootMaster = m_lootMethod == MASTER_LOOT ? m_looterGuid : ObjectGuid::Empty;

        partyUpdate.DifficultySettings = boost::in_place();
        partyUpdate.DifficultySettings->DungeonDifficultyID = m_dungeonDifficulty;
        partyUpdate.DifficultySettings->RaidDifficultyID = m_raidDifficulty;
        partyUpdate.DifficultySettings->LegacyRaidDifficultyID = m_legacyRaidDifficulty;
    }

    // LfgInfos
    if (isLFGGroup())
    {
        uint32 QueueId = sLFGMgr->GetQueueId(m_guid);
        auto dungeon = sLFGMgr->GetLFGDungeon(sLFGMgr->GetDungeon(m_guid, true), player->GetTeam());
        auto lfgState = sLFGMgr->GetState(m_guid, QueueId);
        uint8 flags = 0;
        if (lfgState == lfg::LFG_STATE_FINISHED_DUNGEON || dungeon && dungeon->dbc->Flags & LFG_FLAG_NON_BACKFILLABLE)
            flags |= 2;

        partyUpdate.LfgInfos = boost::in_place();
        partyUpdate.LfgInfos->Slot = sLFGMgr->GetLFGDungeonEntry(sLFGMgr->GetDungeon(m_guid));
        partyUpdate.LfgInfos->MyFlags = flags;
        partyUpdate.LfgInfos->MyPartialClear = sLFGMgr->GetState(m_guid, QueueId) == lfg::LFG_STATE_FINISHED_DUNGEON ? 2 : 0;
        if (dungeon)
            partyUpdate.LfgInfos->MyGearDiff = 1.0f;
        partyUpdate.LfgInfos->MyFirstReward = lfgState != lfg::LFG_STATE_FINISHED_DUNGEON;

        partyUpdate.LfgInfos->MyRandomSlot = [player, QueueId]() -> uint32
        {
            auto const& selectedDungeons = sLFGMgr->GetSelectedDungeons(player->GetGUID(), QueueId);
            if (selectedDungeons.size() == 1)
                if (auto dungeon = sLfgDungeonsStore.LookupEntry(*selectedDungeons.begin()))
                    if (dungeon->TypeID == LFG_TYPE_RANDOM)
                        return dungeon->ID;

            return 0;
        }();

        partyUpdate.LfgInfos->BootCount = 0;
        partyUpdate.LfgInfos->Aborted = false;
        partyUpdate.LfgInfos->MyStrangerCount = 0;
        partyUpdate.LfgInfos->MyKickVoteCount = 0;
    }

    player->SendDirectMessage(partyUpdate.Write());
}

void Group::SendUpdateDestroyGroupToPlayer(Player* player) const
{
    WorldPackets::Party::PartyUpdate partyUpdate;
    partyUpdate.PartyFlags = GROUP_FLAG_DESTROYED;
    partyUpdate.PartyIndex = m_groupCategory;
    partyUpdate.PartyType = GROUP_TYPE_NONE;
    partyUpdate.PartyGUID = m_guid;
    partyUpdate.MyIndex = -1;
    partyUpdate.SequenceNum = player->NextGroupUpdateSequenceNumber(m_groupCategory);
    player->GetSession()->SendPacket(partyUpdate.Write());
}

void Group::UpdatePlayerOutOfRange(Player* player)
{
    if (!player || !player->IsInWorld() || !player->CanContact())
        return;

    WorldPackets::Party::PartyMemberStatseUpdate packet;
    packet.Initialize(player);

    auto p = packet.Write();
    for (auto itr = GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        auto member = itr->getSource();
        if (member && member != player && (!member->IsInMap(player) || !member->IsWithinDist(player, member->GetSightRange(), false)))
            member->SendDirectMessage(p);
    }
}

void Group::BroadcastAddonMessagePacket(WorldPacket const* packet, std::string const& prefix, bool ignorePlayersInBGRaid, int group /*= -1*/, ObjectGuid ignore /*= ObjectGuid::Empty*/)
{
    for (GroupReference* itr = GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* player = itr->getSource();
        if (!player || !player->CanContact() || ignore && player->GetGUID() == ignore || ignorePlayersInBGRaid && player->GetGroup() != this)
            continue;

        if (WorldSession* session = player->GetSession())
            if (session && (group == -1 || itr->getSubGroup() == group))
                if (session->IsAddonRegistered(prefix))
                    player->SendDirectMessage(packet);
    }
}

void Group::BroadcastPacket(const WorldPacket* packet, bool ignorePlayersInBGRaid, int group, ObjectGuid ignore)
{
    for (GroupReference* itr = GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* player = itr->getSource();
        if (!player || !player->CanContact() || (ignore && player->GetGUID() == ignore) || (ignorePlayersInBGRaid && player->GetGroup() != this))
            continue;

        if (group == -1 || itr->getSubGroup() == group)
            player->SendDirectMessage(packet);
    }
}

void Group::BroadcastReadyCheck(WorldPacket const* packet)
{
    for (GroupReference* itr = GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* player = itr->getSource();
        if (player && player->CanContact())
            if (IsLeader(player->GetGUID()) || IsAssistant(player->GetGUID()) || m_groupFlags & GROUP_FLAG_EVERYONE_ASSISTANT)
                player->SendDirectMessage(packet);
    }
}

void Group::OfflineReadyCheck()
{
    bool ready = false;
    for (member_citerator citr = m_memberSlots.begin(); citr != m_memberSlots.end(); ++citr)
    {
        if (!citr->Guid.IsPlayer())
            continue;

        Player* player = ObjectAccessor::FindPlayer(citr->Guid);
        if (!player || !player->GetSession())
        {
            WorldPackets::Party::ReadyCheckResponse response;
            response.PartyGUID = GetGUID();
            response.Player = citr->Guid;
            response.IsReady = ready;
            BroadcastReadyCheck(response.Write());

            m_readyCheckCount++;
        }
    }
}

bool Group::_setMembersGroup(ObjectGuid guid, uint8 group)
{
    auto slot = _getMemberWSlot(guid);
    if (slot == m_memberSlots.end())
        return false;

    slot->Group = group;

    SubGroupCounterIncrease(group);

    if (!isBGGroup() && !isBFGroup())
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GROUP_MEMBER_SUBGROUP);

        stmt->setUInt8(0, group);
        stmt->setUInt64(1, guid.GetCounter());

        CharacterDatabase.Execute(stmt);
    }

    return true;
}

bool Group::SameSubGroup(Player const* member1, Player const* member2) const
{
    if (!member1 || !member2)
        return false;

    if (member1->GetGroup() != this || member2->GetGroup() != this)
        return false;

    return member1->GetSubGroup() == member2->GetSubGroup();
}

// Allows setting sub groups both for online or offline members
void Group::ChangeMembersGroup(ObjectGuid guid, uint8 group)
{
    // Only raid groups have sub groups
    if (!isRaidGroup())
        return;

    // Check if player is really in the raid
    auto slot = _getMemberWSlot(guid);
    if (slot == m_memberSlots.end())
        return;

    // Abort if the player is already in the target sub group
    uint8 prevSubGroup = GetMemberGroup(guid);
    if (prevSubGroup == group)
        return;

    // Update the player slot with the new sub group setting
    slot->Group = group;

    // Increase the counter of the new sub group..
    SubGroupCounterIncrease(group);

    // ..and decrease the counter of the previous one
    SubGroupCounterDecrease(prevSubGroup);

    // Preserve new sub group in database for non-raid groups
    if (!isBGGroup() && !isBFGroup())
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GROUP_MEMBER_SUBGROUP);

        stmt->setUInt8(0, group);
        stmt->setUInt64(1, guid.GetCounter());

        CharacterDatabase.Execute(stmt);
    }

    // In case the moved player is online, update the player object with the new sub group references
    if (Player* player = ObjectAccessor::FindPlayer(guid))
    {
        if (player->GetGroup() == this)
            player->GetGroupRef().setSubGroup(group);
        else
            player->GetOriginalGroupRef().setSubGroup(group);
    }

    // Broadcast the changes to the group
    SendUpdate();
}

// Retrieve the next Round-Roubin player for the group
//
// No update done if loot method is Master or FFA.
//
// If the RR player is not yet set for the group, the first group member becomes the round-robin player.
// If the RR player is set, the next player in group becomes the round-robin player.
//
// If ifneed is true,
//      the current RR player is checked to be near the looted object.
//      if yes, no update done.
//      if not, he loses his turn.
void Group::UpdateLooterGuid(WorldObject* pLootedObject, bool ifneed)
{
    switch (GetLootMethod())
    {
        case MASTER_LOOT:
        case FREE_FOR_ALL:
        case PERSONAL_LOOT:
            return;
        default:
            // round robin style looting applies for all low
            // quality items in each loot method except free for all and master loot
            break;
    }

    ObjectGuid oldLooterGUID = GetLooterGuid();
    auto guid_itr = _getMemberCSlot(oldLooterGUID);
    if (guid_itr != m_memberSlots.end())
    {
        if (ifneed)
        {
            // not update if only update if need and ok
            Player* looter = ObjectAccessor::FindPlayer(guid_itr->Guid);
            if (looter && looter->IsWithinDistInMap(pLootedObject, sWorld->getFloatConfig(CONFIG_GROUP_XP_DISTANCE), false))
                return;
        }
        ++guid_itr;
    }

    // search next after current
    Player* pNewLooter = nullptr;
    for (auto itr = guid_itr; itr != m_memberSlots.end(); ++itr)
    {
        if (!itr->Guid.IsPlayer())
            continue;

        if (Player* player = ObjectAccessor::FindPlayer(itr->Guid))
            if (player->IsWithinDistInMap(pLootedObject, sWorld->getFloatConfig(CONFIG_GROUP_XP_DISTANCE), false))
            {
                pNewLooter = player;
                break;
            }
    }

    if (!pNewLooter)
    {
        // search from start
        for (member_citerator itr = m_memberSlots.begin(); itr != guid_itr; ++itr)
        {
            if (!itr->Guid.IsPlayer())
                continue;
            if (Player* player = ObjectAccessor::FindPlayer(itr->Guid))
                if (player->IsWithinDistInMap(pLootedObject, sWorld->getFloatConfig(CONFIG_GROUP_XP_DISTANCE), false))
                {
                    pNewLooter = player;
                    break;
                }
        }
    }

    if (pNewLooter)
    {
        if (oldLooterGUID != pNewLooter->GetGUID())
        {
            SetLooterGuid(pNewLooter->GetGUID());
            SendUpdate();
        }
    }
    else
    {
        SetLooterGuid(ObjectGuid::Empty);
        SendUpdate();
    }
}

uint8 Group::CanJoinBattlegroundQueue(Battleground const* bgOrTemplate, uint8 bgQueueTypeId, uint32 minPlayerCount, bool isRated, uint32 bracketType, ObjectGuid& errorGuid)
{
    if (isLFGGroup())
        return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_LFG_CANT_USE_BATTLEGROUND;

    auto bgEntry = sBattlemasterListStore.LookupEntry(bgOrTemplate->GetTypeID());
    if (!bgEntry)
        return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEGROUND_JOIN_FAILED;

    auto memberscount = GetMembersCount();
    if (memberscount > bgEntry->MaxPlayers)
        return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEGROUND_NONE;

    auto reference = GetFirstMember()->getSource();
    if (!reference)
        return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEGROUND_JOIN_FAILED;

    auto bracketEntry = sDB2Manager.GetBattlegroundBracketByLevel(bgOrTemplate->GetMapId(), reference->getLevel());
    if (!bracketEntry)
        return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEGROUND_JOIN_FAILED;

    if (bracketType < MS::Battlegrounds::BracketType::Max && !reference->getBracket(bracketType))
        return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEGROUND_JOIN_FAILED;

    if (bgOrTemplate->GetMaxGroupSize() && GetMembersCount() > bgOrTemplate->GetMaxGroupSize())
        return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEFIELD_TEAM_PARTY_SIZE;

    auto team = reference->GetTeam();

    auto bgQueueTypeIdRandom = MS::Battlegrounds::GetBgQueueTypeIdByBgTypeID(MS::Battlegrounds::BattlegroundTypeId::BattlegroundRandom);

    memberscount = 0;
    for (auto itr = GetFirstMember(); itr != nullptr; itr = itr->next(), ++memberscount)
    {
        auto member = itr->getSource();
        if (!member)
            return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEGROUND_JOIN_FAILED;

        if (member->GetTeam() != team)
        {
            errorGuid = member->GetGUID();
            return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEGROUND_JOIN_TIMED_OUT;
        }

        if (sDB2Manager.GetBattlegroundBracketByLevel(bracketEntry->MapID, member->getLevel()) != bracketEntry)
            return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEGROUND_JOIN_RANGE_INDEX;

        if (member->InBattlegroundQueueForBattlegroundQueueType(bgQueueTypeId))
            return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEGROUND_JOIN_FAILED;            // not blizz-like

        if (member->InBattlegroundQueueForBattlegroundQueueType(bgQueueTypeIdRandom))
            return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_IN_RANDOM_BG;

        if (bgOrTemplate->GetTypeID() == MS::Battlegrounds::BattlegroundTypeId::BattlegroundRandom && member->InBattlegroundQueue())
            return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_IN_NON_RANDOM_BG;

        if (bgOrTemplate->GetTypeID() == MS::Battlegrounds::BattlegroundTypeId::ArenaAll && !member->CanJoinToBattleground(bgOrTemplate->IsArena() || bgOrTemplate->IsSkirmish() ? MS::Battlegrounds::IternalPvpTypes::Arena : MS::Battlegrounds::IternalPvpTypes::Battleground))
            return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_GROUP_JOIN_BATTLEGROUND_DESERTERS;

        if (bgOrTemplate->GetTypeID() == MS::Battlegrounds::BattlegroundTypeId::ArenaAll && !GetMaxCountOfRolesForArenaQueue(ROLES_HEALER))
            return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEGROUND_JOIN_TOO_MANY_HEALERS;

        if (bgOrTemplate->GetTypeID() == MS::Battlegrounds::BattlegroundTypeId::ArenaAll && !GetMaxCountOfRolesForArenaQueue(ROLES_TANK))
            return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEGROUND_JOIN_TOO_MANY_TANKS;

        if (!member->HasFreeBattlegroundQueueId())
            return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEGROUND_TOO_MANY_QUEUES;        // not blizz-like

        if (member->isUsingLfg())
            return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_LFG_CANT_USE_BATTLEGROUND;
    }

    if ((bgOrTemplate->IsArena() || isRated) && memberscount != minPlayerCount && !sBattlegroundMgr->isTesting())
        return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_ARENA_TEAM_PARTY_SIZE;

    return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEGROUND_NONE;
}

//===================================================
//============== Roll ===============================
//===================================================

void Group::SetDungeonDifficultyID(Difficulty difficulty)
{
    m_dungeonDifficulty = difficulty;
    if (!isBGGroup() && !isBFGroup() && difficulty != DIFFICULTY_MYTHIC_KEYSTONE)
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GROUP_DIFFICULTY);

        stmt->setUInt8(0, uint8(m_dungeonDifficulty != DIFFICULTY_MYTHIC_KEYSTONE ? m_dungeonDifficulty : DIFFICULTY_MYTHIC_DUNGEON));
        stmt->setUInt32(1, m_dbStoreId);

        CharacterDatabase.Execute(stmt);
    }

    for (GroupReference* itr = GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* player = itr->getSource();
        if (!player->GetSession())
            continue;

        player->SetDungeonDifficultyID(difficulty);
        player->SendDungeonDifficulty();
    }
}

void Group::SetRaidDifficultyID(Difficulty difficulty)
{
    m_raidDifficulty = difficulty;
    if (!isBGGroup() && !isBFGroup())
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GROUP_RAID_DIFFICULTY);

        stmt->setUInt8(0, uint8(m_raidDifficulty));
        stmt->setUInt32(1, m_dbStoreId);

        CharacterDatabase.Execute(stmt);
    }

    for (GroupReference* itr = GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* player = itr->getSource();
        if (!player->GetSession())
            continue;

        player->SetRaidDifficultyID(difficulty);
        player->SendRaidDifficulty(false);
    }
}

void Group::SetLegacyRaidDifficultyID(Difficulty difficulty)
{
    m_legacyRaidDifficulty = difficulty;
    if (!isBGGroup() && !isBFGroup())
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GROUP_LEGACY_RAID_DIFFICULTY);

        stmt->setUInt8(0, uint8(m_legacyRaidDifficulty));
        stmt->setUInt32(1, m_dbStoreId);

        CharacterDatabase.Execute(stmt);
    }

    for (GroupReference* itr = GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* player = itr->getSource();
        if (!player->GetSession())
            continue;

        player->SetLegacyRaidDifficultyID(difficulty);
        player->SendRaidDifficulty(true);
    }
}

Difficulty Group::GetDifficultyID(MapEntry const* mapEntry) const
{
    if (!mapEntry->IsRaid())
        return m_dungeonDifficulty;

    MapDifficultyEntry const* defaultDifficulty = sDB2Manager.GetDefaultMapDifficulty(mapEntry->ID);
    if (!defaultDifficulty)
        return m_legacyRaidDifficulty;

    DifficultyEntry const* difficulty = sDifficultyStore.LookupEntry(defaultDifficulty->DifficultyID);
    if (!difficulty || difficulty->Flags & DIFFICULTY_FLAG_LEGACY)
        return m_legacyRaidDifficulty;

    return m_raidDifficulty;
}

Difficulty Group::GetDungeonDifficultyID() const
{
    return m_dungeonDifficulty;
}

Difficulty Group::GetRaidDifficultyID() const
{
    return m_raidDifficulty;
}

Difficulty Group::GetLegacyRaidDifficultyID() const
{
    return m_legacyRaidDifficulty;
}

void Group::ResetInstances(uint8 method, bool isRaid, bool isLegacy, Player* SendMsgTo)
{
    if (isBGGroup() || isBFGroup())
        return;

    // method can be INSTANCE_RESET_ALL, INSTANCE_RESET_CHANGE_DIFFICULTY, INSTANCE_RESET_GROUP_DISBAND

    // we assume that when the difficulty changes, all instances that can be reset will be
    Difficulty diff = GetDungeonDifficultyID();
    uint8 boundType = sObjectMgr->GetboundTypeFromDifficulty(diff);
    if (isRaid)
    {
        if (!isLegacy)
            diff = GetRaidDifficultyID();
        else
            diff = GetLegacyRaidDifficultyID();
    }

    std::lock_guard<std::recursive_mutex> _lock(m_bound_lock);
    for (auto itr = m_boundInstances[boundType].begin(); itr != m_boundInstances[boundType].end();)
    {
        InstanceSave* instanceSave = itr->second.save;
        const MapEntry* entry = sMapStore.LookupEntry(itr->first);
        if (!entry || entry->IsRaid() != isRaid || !instanceSave->CanReset() && method != INSTANCE_RESET_GROUP_DISBAND)
        {
            ++itr;
            continue;
        }

        if (method == INSTANCE_RESET_ALL)
        {
            // the "reset all instances" method can only reset normal maps
            if (entry->IsRaid() || diff == DIFFICULTY_HEROIC)
            {
                ++itr;
                continue;
            }
        }

        bool isEmpty = true;
        // if the map is loaded, reset it
        Map* map = sMapMgr->FindMap(instanceSave->GetMapId(), instanceSave->GetInstanceId());
        if (map && map->IsDungeon() && !(method == INSTANCE_RESET_GROUP_DISBAND && !instanceSave->CanReset()))
        {
            if (instanceSave->CanReset() && map->ToInstanceMap())
                isEmpty = map->ToInstanceMap()->Reset(method);
            else
                isEmpty = !map->HavePlayers();
        }

        if (SendMsgTo)
        {
            if (isEmpty)
                SendMsgTo->SendResetInstanceSuccess(instanceSave->GetMapId());
            else
                SendMsgTo->SendResetInstanceFailed(ResetFailedReason::FAILED, instanceSave->GetMapId());
        }

        if (isEmpty || method == INSTANCE_RESET_GROUP_DISBAND || method == INSTANCE_RESET_CHANGE_DIFFICULTY)
        {
            // do not reset the instance, just unbind if others are permanently bound to it
            if (instanceSave->CanReset())
                instanceSave->DeleteFromDB();
            else
            {
                PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GROUP_INSTANCE_BY_INSTANCE);

                stmt->setUInt32(0, instanceSave->GetInstanceId());

                CharacterDatabase.Execute(stmt);
            }


            // i don't know for sure if hash_map iterators
            m_boundInstances[boundType].erase(itr);
            itr = m_boundInstances[boundType].begin();
            // this unloads the instance save unless online players are bound to it
            // (eg. permanent binds or GM solo binds)
            instanceSave->RemoveGroup(this);
        }
        else
            ++itr;
    }
}

InstanceGroupBind* Group::GetBoundInstance(Player* player)
{
    return GetBoundInstance(sMapStore.LookupEntry(player->GetMapId()));
}

InstanceGroupBind* Group::GetBoundInstance(Map* aMap)
{
    return GetBoundInstance(aMap->GetEntry());
}

InstanceGroupBind* Group::GetBoundInstance(MapEntry const* mapEntry)
{
    if (!mapEntry || !mapEntry->IsDungeon())
        return nullptr;

    return GetBoundInstance(GetDifficultyID(mapEntry), mapEntry->ID);
}

InstanceGroupBind* Group::GetBoundInstance(Difficulty difficulty, uint32 mapId)
{
    // some instances only have one difficulty
    sDB2Manager.GetDownscaledMapDifficultyData(mapId, difficulty);

    uint8 boundType = sObjectMgr->GetboundTypeFromDifficulty(difficulty);

    auto itr = m_boundInstances[boundType].find(mapId);
    if (itr != m_boundInstances[boundType].end())
        return &itr->second;
    return nullptr;
}

InstanceGroupBind* Group::BindToInstance(InstanceSave* save, bool permanent, bool load)
{
    if (!save || isBGGroup() || isBFGroup())
        return nullptr;

    std::lock_guard<std::recursive_mutex> _lock(m_bound_lock);
    uint8 boundType = sObjectMgr->GetboundTypeFromDifficulty(save->GetDifficultyID());
    InstanceGroupBind& bind = m_boundInstances[boundType][save->GetMapId()];
    if (save->CanBeSave())
    {
        save->SetPerm(permanent);
        if (!load && (!bind.save || permanent != bind.perm || save != bind.save))
            UpdateInstance(save);
    }
    else
        permanent = false;

    if (bind.save != save)
    {
        if (bind.save)
            bind.save->RemoveGroup(this);
        save->AddGroup(this);
    }

    bind.save = save;
    bind.perm = permanent;
    if (!load)
        TC_LOG_DEBUG(LOG_FILTER_MAPS, "Group::BindToInstance: Group (guid: %u, storage id: %u) is now bound to map %d, instance %d, difficulty %d",
        GetGUIDLow(), m_dbStoreId, save->GetMapId(), save->GetInstanceId(), save->GetDifficultyID());

    return &bind;
}

void Group::UnbindInstance(uint32 mapid, uint8 difficulty, bool unload)
{
    std::lock_guard<std::recursive_mutex> _lock(m_bound_lock);
    uint8 boundType = sObjectMgr->GetboundTypeFromDifficulty(difficulty);
    auto itr = m_boundInstances[boundType].find(mapid);
    if (itr != m_boundInstances[boundType].end())
    {
        if (!unload)
        {
            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GROUP_INSTANCE_BY_GUID);
            stmt->setUInt32(0, m_dbStoreId);
            stmt->setUInt32(1, itr->second.save->GetInstanceId());
            CharacterDatabase.Execute(stmt);
        }

        itr->second.save->RemoveGroup(this);                // save can become invalid
        m_boundInstances[boundType].erase(itr);
    }
}

void Group::UpdateInstance(InstanceSave* save)
{
    if (!save || !save->GetMapEntry() || save->GetMapEntry()->IsGarrison() || save->GetMapEntry()->CanCreatedZone())
        return;

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_GROUP_INSTANCE);

    stmt->setUInt64(0, m_dbStoreId);
    stmt->setUInt32(1, save->GetInstanceId());
    stmt->setUInt16(2, save->GetMapId());
    stmt->setUInt8(3, save->GetDifficultyID());
    stmt->setBool(4, save->GetPerm());
    stmt->setUInt32(5, save->GetCompletedEncounterMask());
    stmt->setString(6, save->GetData());
    stmt->setUInt32(7, save->GetResetTime());

    CharacterDatabase.Execute(stmt);
}

void Group::_homebindIfInstance(Player* player)
{
    if (player && !player->isGameMaster() && sMapStore.LookupEntry(player->GetMapId())->IsDungeon())
        player->m_InstanceValid = false;
}

void Group::BroadcastGroupUpdate()
{
    // FG: HACK: force flags update on group leave - for values update hack
    // -- not very efficient but safe
    for (member_citerator citr = m_memberSlots.begin(); citr != m_memberSlots.end(); ++citr)
    {
        if (!citr->Guid.IsPlayer())
            continue;
        Player* pp = ObjectAccessor::FindPlayer(citr->Guid);
        if (pp && pp->IsInWorld())
        {
            pp->ForceValuesUpdateAtIndex(UNIT_FIELD_BYTES_2);
            pp->ForceValuesUpdateAtIndex(UNIT_FIELD_FACTION_TEMPLATE);
            TC_LOG_DEBUG(LOG_FILTER_GENERAL, "-- Forced group value update for '%s'", pp->GetName());
        }
    }
}

void Group::SetReadyCheckCount(uint8 count)
{
    m_readyCheckCount = count;
}

uint8 Group::GetReadyCheckCount() const
{
    return m_readyCheckCount;
}

uint8 Group::GetGroupFlags() const
{
    return m_groupFlags;
}

GroupCategory Group::GetGroupCategory() const
{
    return m_groupCategory;
}

void Group::ResetMaxEnchantingLevel()
{
    m_maxEnchantingLevel = 0;
    for (member_citerator citr = m_memberSlots.begin(); citr != m_memberSlots.end(); ++citr)
    {
        if (!citr->Guid.IsPlayer())
            continue;
        Player* pMember = ObjectAccessor::FindPlayer(citr->Guid);
        if (pMember && m_maxEnchantingLevel < pMember->GetSkillValue(SKILL_ENCHANTING))
            m_maxEnchantingLevel = pMember->GetSkillValue(SKILL_ENCHANTING);
    }
}

void Group::SetLootMethod(LootMethod method)
{
    m_lootMethod = method;
}

void Group::SetLooterGuid(ObjectGuid const& guid)
{
    m_looterGuid = guid;
}

void Group::SetLootThreshold(ItemQualities threshold)
{
    m_lootThreshold = threshold;
}

void Group::SetLfgRoles(ObjectGuid guid, const uint8 roles)
{
    auto slot = _getMemberWSlot(guid);
    if (slot == m_memberSlots.end())
        return;

    slot->Roles = roles;

    if (sLFGListMgr->IsGroupQueued(this))
        sLFGListMgr->SendSocialQueueUpdateNotify(sLFGListMgr->GetEntrybyGuidLow(GetGUIDLow()));
}

uint8 Group::GetLfgRoles(ObjectGuid guid)
{
    auto slot = _getMemberWSlot(guid);
    if (slot == m_memberSlots.end())
        return 0;

    return slot->Roles;
}

bool Group::IsFull() const
{
    return isRaidGroup() ? m_memberSlots.size() >= MAX_RAID_SIZE : m_memberSlots.size() >= MAX_GROUP_SIZE;
}

bool Group::isLFGGroup() const
{
    return (m_groupFlags & GROUP_FLAG_LFG) != 0;
}

bool Group::isRaidGroup() const
{
    return (m_groupFlags & GROUP_FLAG_RAID) != 0;
}

bool Group::isBGGroup() const
{
    return m_bgGroup != nullptr;
}

bool Group::isArenaGroup() const
{
    return m_bgGroup && m_bgGroup->IsArena();
}

bool Group::isBFGroup() const
{
    return m_bfGroup != nullptr;
}

bool Group::IsCreated() const
{
    return GetMembersCount() > 0;
}

bool Group::IsHomeGroup() const
{
    return !isLFGGroup() && (!isBGGroup() || isArenaGroup()) && !isBFGroup();
}

ObjectGuid Group::GetLeaderGUID() const
{
    return m_leaderGuid;
}

ObjectGuid Group::GetGUID() const
{
    return m_guid;
}

uint32 Group::GetGUIDLow() const
{
    return m_guid.GetGUIDLow();
}

const char * Group::GetLeaderName() const
{
    return m_leaderName.c_str();
}

LootMethod Group::GetLootMethod() const
{
    return m_lootMethod;
}

ObjectGuid Group::GetLooterGuid() const
{
    return m_looterGuid;
}

ItemQualities Group::GetLootThreshold() const
{
    return m_lootThreshold;
}

uint32 Group::GetDbStoreId() const
{
    return m_dbStoreId;
}

bool Group::IsMember(ObjectGuid guid) const
{
    return _getMemberCSlot(guid) != m_memberSlots.end();
}

bool Group::IsLeader(ObjectGuid guid) const
{
    return GetLeaderGUID() == guid;
}

ObjectGuid Group::GetMemberGUID(std::string const& name)
{
    for (member_citerator itr = m_memberSlots.begin(); itr != m_memberSlots.end(); ++itr)
        if (itr->Name == name)
            return itr->Guid;

    return ObjectGuid::Empty;
}

bool Group::IsAssistant(ObjectGuid guid) const
{
    auto mslot = _getMemberCSlot(guid);
    if (mslot == m_memberSlots.end())
        return false;

    return mslot->Flags & MEMBER_FLAG_ASSISTANT;
}

bool Group::SameSubGroup(ObjectGuid guid1, ObjectGuid guid2) const
{
    auto mslot2 = _getMemberCSlot(guid2);
    if (mslot2 == m_memberSlots.end())
        return false;
    return SameSubGroup(guid1, &*mslot2);
}

bool Group::SameSubGroup(ObjectGuid guid1, MemberSlot const* slot2) const
{
    auto mslot1 = _getMemberCSlot(guid1);
    if (mslot1 == m_memberSlots.end() || !slot2)
        return false;

    return mslot1->Group == slot2->Group;
}

bool Group::HasFreeSlotSubGroup(uint8 subgroup) const
{
    return m_subGroupsCounts && m_subGroupsCounts[subgroup] < MAX_GROUP_SIZE;
}

Group::MemberSlotList const& Group::GetMemberSlots() const
{
    return m_memberSlots;
}

GroupReference* Group::GetFirstMember()
{
    return m_memberMgr.getFirst();
}

GroupReference const* Group::GetFirstMember() const
{
    return m_memberMgr.getFirst();
}

uint32 Group::GetMembersCount() const
{
    return m_memberSlots.size();
}

uint8 Group::GetMemberGroup(ObjectGuid guid) const
{
    auto mslot = _getMemberCSlot(guid);
    if (mslot == m_memberSlots.end())
        return MAX_RAID_SUBGROUPS + 1;

    return mslot->Group;
}

void Group::SetBattlegroundGroup(Battleground* bg)
{
    m_bgGroup = bg;
}

void Group::SetBattlefieldGroup(Battlefield *bg)
{
    m_bfGroup = bg;
}

void Group::setGroupMemberRole(ObjectGuid guid, uint32 role)
{
    for (auto& itr : m_memberSlots)
    {
        if (itr.Guid == guid)
        {
            itr.Roles = role;
            break;
        }
    }

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GROUP_MEMBER_ROLE);
    if (stmt != nullptr)
    {
        stmt->setUInt8(0, role);
        stmt->setUInt64(1, guid.GetCounter());
        CharacterDatabase.Execute(stmt);
    }
}

void Group::SetGroupMemberFlag(ObjectGuid guid, bool apply, GroupMemberFlags flag)
{
    // Assistants, main assistants and main tanks are only available in raid groups
    if (!isRaidGroup())
        return;

    // Check if player is really in the raid
    auto slot = _getMemberWSlot(guid);
    if (slot == m_memberSlots.end())
        return;

    // Do flag specific actions, e.g ensure uniqueness
    switch (flag)
    {
        case MEMBER_FLAG_MAINASSIST:
            RemoveUniqueGroupMemberFlag(MEMBER_FLAG_MAINASSIST);         // Remove main assist flag from current if any.
            break;
        case MEMBER_FLAG_MAINTANK:
            RemoveUniqueGroupMemberFlag(MEMBER_FLAG_MAINTANK);           // Remove main tank flag from current if any.
            break;
        case MEMBER_FLAG_ASSISTANT:
            break;
        default:
            return;                                                      // This should never happen
    }

    // Switch the actual flag
    ToggleGroupMemberFlag(slot, flag, apply);

    // Preserve the new setting in the db
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GROUP_MEMBER_FLAG);

    stmt->setUInt8(0, slot->Flags);
    stmt->setUInt64(1, guid.GetCounter());

    CharacterDatabase.Execute(stmt);

    // Broadcast the changes to the group
    SendUpdate();
}


void Group::ErraseRollbyRealSlot(uint8 slot, Loot* loot)
{
    for (auto iter = RollId.begin(); iter != RollId.end(); ++iter)
    {
        if ((*iter)->itemSlot == slot && loot == (*iter)->getLoot() && (*iter)->isValid())
        {
            delete *iter;
            RollId.erase(iter);
            break;
        }
    }
}

Group::Rolls::iterator Group::GetRoll(uint8 _aoeSlot)
{
    for (auto iter = RollId.begin(); iter != RollId.end(); ++iter)
        if ((*iter)->aoeSlot == _aoeSlot && (*iter)->isValid())
            return iter;
    return RollId.end();
}

void Group::LinkMember(GroupReference* pRef)
{
    m_memberMgr.insertFirst(pRef);
}

void Group::DelinkMember(ObjectGuid guid)
{
    GroupReference* ref = m_memberMgr.getFirst();
    while (ref)
    {
        GroupReference* nextRef = ref->next();
        if (ref->getSource()->GetGUID() == guid)
        {
            ref->unlink();
            break;
        }
        ref = nextRef;
    }
}

Group::BoundInstancesMap& Group::GetBoundInstances(Difficulty difficulty)
{
    return m_boundInstances[sObjectMgr->GetboundTypeFromDifficulty(difficulty)];
}

void Group::_initRaidSubGroupsCounter()
{
    // Sub group counters initialization
    if (!m_subGroupsCounts)
        m_subGroupsCounts = new uint8[MAX_RAID_SUBGROUPS];

    memset(static_cast<void*>(m_subGroupsCounts), 0, MAX_RAID_SUBGROUPS * sizeof(uint8));

    for (member_citerator itr = m_memberSlots.begin(); itr != m_memberSlots.end(); ++itr)
        ++m_subGroupsCounts[itr->Group];
}

Group::member_citerator Group::_getMemberCSlot(ObjectGuid Guid) const
{
    std::lock_guard<std::recursive_mutex> _lock(const_cast<Group*>(this)->m_lock);
    for (auto itr = m_memberSlots.begin(); itr != m_memberSlots.end(); ++itr)
        if (itr->Guid == Guid)
            return itr;

    return m_memberSlots.end();
}

Group::member_witerator Group::_getMemberWSlot(ObjectGuid Guid)
{
    for (auto itr = m_memberSlots.begin(); itr != m_memberSlots.end(); ++itr)
        if (itr->Guid == Guid)
            return itr;

    return m_memberSlots.end();
}

void Group::SubGroupCounterIncrease(uint8 subgroup)
{
    if (m_subGroupsCounts)
        ++m_subGroupsCounts[subgroup];
}

void Group::SubGroupCounterDecrease(uint8 subgroup)
{
    if (m_subGroupsCounts)
        --m_subGroupsCounts[subgroup];
}

void Group::RemoveUniqueGroupMemberFlag(GroupMemberFlags flag)
{
    for (auto& itr : m_memberSlots)
        if (itr.Flags & flag)
            itr.Flags &= ~flag;
}

void Group::ToggleGroupMemberFlag(member_witerator slot, uint8 flag, bool apply)
{
    if (apply)
        slot->Flags |= flag;
    else
        slot->Flags &= ~flag;
}

bool Group::IsGuildGroup(ObjectGuid const& guildId, bool AllInSameMap, bool AllInSameInstanceId)
{
    uint32 mapId = 0;
    uint32 InstanceId = 0;
    std::vector<Player*> members;
    // First we populate the array
    for (GroupReference *itr = GetFirstMember(); itr != nullptr; itr = itr->next()) // Loop trought all members
        if (Player* player = itr->getSource())
            if (player->IsInWorld())
                if (player->GetGuildId() == guildId.GetCounter()) // Check if it has a guild
                    members.push_back(player);

    bool ret = false;
    
    auto count = members.size();
    for (auto player : members) // Iterate through players
    {
        if (player)
        {
            if (!player->IsInWorld())
                continue;

            if (mapId == 0)
                mapId = player->GetMapId();

            if (InstanceId == 0)
                InstanceId = player->GetInstanceId();

            Map* map = player->GetMap();
            if (count >= map->GetMapMaxPlayers() * 0.8f)
                ret = true;

            if (map->IsNonRaidDungeon() && !ret)
                if (count >= 3)
                    ret = true;

            if (map->IsBattleArena() && !ret)
                if (count == GetMembersCount())
                    ret = true;

            if (map->IsBattleground() && !ret)
                if (Battleground* bg = player->GetBattleground())
                    if (count >= uint32(bg->GetMaxPlayers() * 0.8f))
                        ret = true;

            if (AllInSameMap && mapId != player->GetMapId())
                return false;

            if (AllInSameInstanceId && InstanceId != player->GetInstanceId())
                return false;
        }
    }

    return ret;
}

void Group::UpdateGuildAchievementCriteria(CriteriaTypes type, uint32 miscValue1, uint32 miscValue2, uint32 miscValue3, Unit* pUnit, WorldObject* pRewardSource)
{
    // We will update criteria for each guild in grouplist but only once
    std::list<ObjectGuid::LowType> guildList;
    for (GroupReference *itr = GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        if (Player *pPlayer = itr->getSource())
        {
            // Check for reward
            if (pRewardSource)
            {
                if (!pPlayer->IsAtGroupRewardDistance(pRewardSource))
                    continue;

                if (!pPlayer->isAlive())
                    continue;
            }

            ObjectGuid::LowType guildId = pPlayer->GetGuildId();
            if (!guildId)
                continue;

            if (!guildList.empty())
            {
                // if we already have any guild in list
                // then check new guild
                bool bUnique = true;
                for (std::list<ObjectGuid::LowType>::const_iterator itr2 = guildList.begin(); itr2 != guildList.end(); ++itr2)
                {
                    if (*itr2 == guildId)
                    {
                        bUnique = false;
                        break;
                    }
                }
                // If we have already rewarded current guild then continue
                // else update criteria 
                if (bUnique && guildId)
                {
                    guildList.push_back(guildId);
                    //if (Guild* pGuild = sGuildMgr->GetGuildById(guildId))
                    //pGuild->GetAchievementMgr().UpdateAchievementCriteria(type, miscValue1, miscValue2, miscValue3, pUnit, pPlayer);
                }
            }
            else
            {
                // If that's first guild in list
                // then add to the list and update criteria
                guildList.push_back(guildId);
                //if (Guild* pGuild = sGuildMgr->GetGuildById(guildId))
                //pGuild->GetAchievementMgr().UpdateAchievementCriteria(type, miscValue1, miscValue2, miscValue3, pUnit, pPlayer);
            }
        }
    }
}

bool Group::leaderInstanceCheckFail()
{
    if (Player const* leader = ObjectAccessor::FindPlayer(GetLeaderGUID()))
    {
        if (!leader->InInstance())
        {
            for (member_citerator citr = m_memberSlots.begin(); citr != m_memberSlots.end(); ++citr)
                if (Player const* member = ObjectAccessor::FindPlayer(citr->Guid))
                    if (member->InInstance())
                        return true;
        }
    }

    return false;
}

uint32 Group::GetAverageMMR(uint8 bracket) const
{
    uint32 matchMakerRating = 0;
    uint32 playerDivider = 0;

    for (const auto& itr : m_memberSlots)
    {
        if (Player const* member = ObjectAccessor::FindPlayer(itr.Guid))
        {
            matchMakerRating += member->getBracket(bracket)->getMMV();
            ++playerDivider;
        }
    }

    // x/0 = crash
    if (playerDivider == 0)
        playerDivider = 1;

    matchMakerRating /= playerDivider;

    return matchMakerRating;
}

ItemQualities Group::GetThreshold() const
{
    return m_lootThreshold;
}

uint32 Group::GetTeam() const
{
    return _team;
}

void Group::AddRaidMarker(uint8 markerId, uint32 mapId, float positionX, float positionY, float positionZ, ObjectGuid transportGuid)
{
    if (markerId >= RAID_MARKERS_COUNT || m_markers[markerId])
        return;

    m_activeMarkers |= 1 << markerId;
    m_markers[markerId] = Trinity::make_unique<RaidMarker>(mapId, positionX, positionY, positionZ, transportGuid);
    SendRaidMarkersChanged();
}

void Group::DeleteRaidMarker(uint8 markerId)
{
    if (markerId > RAID_MARKERS_COUNT)
        return;

    for (uint8 i = 0; i < RAID_MARKERS_COUNT; i++)
        if (m_markers[i] && (markerId == i || markerId == RAID_MARKERS_COUNT))
        {
            m_markers[i] = nullptr;
            m_activeMarkers &= ~(1 << i);
        }

    SendRaidMarkersChanged();
}

void Group::SendRaidMarkersChanged(WorldSession* session, int8 partyIndex)
{
    WorldPackets::Party::RaidMarkersChanged packet;

    packet.PartyIndex = partyIndex;
    packet.ActiveMarkers = m_activeMarkers;

    for (uint8 i = 0; i < RAID_MARKERS_COUNT; i++)
        if (m_markers[i])
            packet.RaidMarkers.push_back(m_markers[i].get());

    if (session)
        session->SendPacket(packet.Write());
    else
        BroadcastPacket(packet.Write(), false);
}

bool Group::InChallenge()
{
    return m_dungeonDifficulty == DIFFICULTY_MYTHIC_KEYSTONE;
}

bool Group::GetMaxCountOfRolesForArenaQueue(uint8 role)
{
    uint8 count = 0;
   
    for (auto itr = GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        if (auto plr = itr->getSource())
        {
            if (plr->GetSpecializationRole() == role)
            {
                ++count;
                if (count > 1)
                    return false;
            }
        }
    }
    return true;
}
