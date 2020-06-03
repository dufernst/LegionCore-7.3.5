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

#include "SocialMgr.h"

#include "DatabaseEnv.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "Player.h"
#include "World.h"
#include "Util.h"
#include "AccountMgr.h"
#include "SocialPackets.h"
#include "ObjectMgr.h"

FriendInfo::FriendInfo(uint8 flags) : Status(FRIEND_STATUS_OFFLINE), Area(0), LfgListActivityID(0), Level(0), Class(0), Flags(flags)
{
}

FriendInfo::FriendInfo(uint8 flags, std::string const& note) : Status(FRIEND_STATUS_OFFLINE), Area(0), LfgListActivityID(0), Note(note), Level(0), Class(0), Flags(flags)
{
}

PlayerSocial::PlayerSocial()
{
    m_playerGUID.Clear();
}

PlayerSocial::~PlayerSocial()
{
    m_playerSocialMap.clear();
}

uint32 PlayerSocial::GetNumberOfSocialsWithFlag(SocialFlag flag)
{
    uint32 counter = 0;
    for (PlayerSocialMap::iterator itr = m_playerSocialMap.begin(); itr != m_playerSocialMap.end(); ++itr)
        if (itr->second.Flags & flag)
            ++counter;

    return counter;
}

bool PlayerSocial::AddToSocialList(ObjectGuid const& friendGuid, SocialFlag flag)
{
    // check client limits
    if (GetNumberOfSocialsWithFlag(flag) >= (((flag & SOCIAL_FLAG_FRIEND) != 0) ? SOCIALMGR_FRIEND_LIMIT : SOCIALMGR_IGNORE_LIMIT))
        return false;

    std::lock_guard<std::recursive_mutex> guard(m_social_lock);
    if (PlayerSocialMap::guarded_ptr itr = m_playerSocialMap.get(friendGuid))
    {
        itr->second.Flags |= flag;

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ADD_CHARACTER_SOCIAL_FLAGS);

        stmt->setUInt8(0, itr->second.Flags);
        stmt->setUInt64(1, GetPlayerGUID().GetCounter());
        stmt->setUInt64(2, friendGuid.GetCounter());

        CharacterDatabase.Execute(stmt);
    }
    else
    {
        m_playerSocialMap.emplace(friendGuid, flag);

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHARACTER_SOCIAL);

        stmt->setUInt64(0, GetPlayerGUID().GetCounter());
        stmt->setUInt64(1, friendGuid.GetCounter());
        stmt->setUInt8(2, flag);

        CharacterDatabase.Execute(stmt);
    }
    return true;
}

void PlayerSocial::RemoveFromSocialList(ObjectGuid const& friendGuid, SocialFlag flag)
{
    PlayerSocialMap::guarded_ptr itr = m_playerSocialMap.get(friendGuid);
    if (!itr)                     // not exist
        return;

    std::lock_guard<std::recursive_mutex> guard(m_social_lock);
    itr->second.Flags &= ~flag;
    if (!itr->second.Flags)
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_SOCIAL);

        stmt->setUInt64(0, GetPlayerGUID().GetCounter());
        stmt->setUInt64(1, friendGuid.GetCounter());

        CharacterDatabase.Execute(stmt);

        m_playerSocialMap.erase(itr->first);
    }
    else
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_REM_CHARACTER_SOCIAL_FLAGS);

        stmt->setUInt8(0, itr->second.Flags);
        stmt->setUInt64(1, GetPlayerGUID().GetCounter());
        stmt->setUInt64(2, friendGuid.GetCounter());

        CharacterDatabase.Execute(stmt);
    }
}

void PlayerSocial::SetFriendNote(ObjectGuid const& friendGuid, std::string note)
{
    PlayerSocialMap::guarded_ptr itr = m_playerSocialMap.get(friendGuid);
    if (!itr)                     // not exist
        return;

    utf8truncate(note, 48);                                  // DB and client size limitation

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHARACTER_SOCIAL_NOTE);

    stmt->setString(0, note);
    stmt->setUInt64(1, GetPlayerGUID().GetCounter());
    stmt->setUInt64(2, friendGuid.GetCounter());

    CharacterDatabase.Execute(stmt);

    itr->second.Note = note;
}

void PlayerSocial::SendSocialList(Player* player, uint32 flags)
{
    if (!player)
        return;

    ASSERT(player);

    WorldPackets::Social::ContactList contactList;
    contactList.Flags = flags;

    std::lock_guard<std::recursive_mutex> guard(m_social_lock);
    for (auto& v : m_playerSocialMap)
    {
        if (!(v.second.Flags & flags))
            continue;

        sSocialMgr->GetFriendInfo(player, v.first, v.second);

        WorldPackets::Social::ContactInfo info;
        info.Guid = v.first;
        info.WowAccountGuid = ObjectGuid::Create<HighGuid::WowAccount>(ObjectMgr::GetPlayerAccountIdByGUID(v.first));
        info.VirtualRealmAddr = GetVirtualRealmAddress();
        info.NativeRealmAddr = GetVirtualRealmAddress();
        info.TypeFlags = v.second.Flags;
        info.Notes = v.second.Note;
        info.Status = v.second.Status;
        info.AreaID = v.second.Area;
        info.Level = v.second.Level;
        info.ClassID = v.second.Class;
        contactList.Contacts.emplace_back(info);

        // client's friends list and ignore list limit
        if (contactList.Contacts.size() >= (((flags & SOCIAL_FLAG_FRIEND) != 0) ? SOCIALMGR_FRIEND_LIMIT : SOCIALMGR_IGNORE_LIMIT))
            break;
    }

    player->SendDirectMessage(contactList.Write());
}

bool PlayerSocial::_HasContact(ObjectGuid const& guid, SocialFlag flags)
{
    PlayerSocialMap::guarded_ptr itr = m_playerSocialMap.get(guid);
    if (itr)
        return (itr->second.Flags & flags) != 0;

    return false;
}

bool PlayerSocial::HasFriend(ObjectGuid const& friendGuid)
{
    return _HasContact(friendGuid, SOCIAL_FLAG_FRIEND);
}

bool PlayerSocial::HasIgnore(ObjectGuid const& ignoreGuid)
{
    return _HasContact(ignoreGuid, SOCIAL_FLAG_IGNORED);
}

SocialMgr::SocialMgr()
{
}

SocialMgr::~SocialMgr()
{
}

SocialMgr* SocialMgr::instance()
{
    static SocialMgr instance;
    return &instance;
}

void SocialMgr::GetFriendInfo(Player* player, ObjectGuid const& friendGUID, FriendInfo &friendInfo)
{
    if (!player)
        return;

    friendInfo.Status = FRIEND_STATUS_OFFLINE;
    friendInfo.Area = 0;
    friendInfo.Level = 0;
    friendInfo.Class = 0;

    Player* pFriend = ObjectAccessor::FindPlayer(friendGUID);
    if (!pFriend)
        return;

    uint32 team = player->GetTeam();
    AccountTypes security = player->GetSession()->GetSecurity();
    bool allowTwoSideWhoList = sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_WHO_LIST);
    AccountTypes gmLevelInWhoList = AccountTypes(sWorld->getIntConfig(CONFIG_GM_LEVEL_IN_WHO_LIST));

    std::lock_guard<std::recursive_mutex> guard(player->GetSocial()->m_social_lock);
    if (PlayerSocialMap::guarded_ptr itr = player->GetSocial()->m_playerSocialMap.get(friendGUID))
        friendInfo.Note = itr->second.Note;

    // PLAYER see his team only and PLAYER can't see MODERATOR, GAME MASTER, ADMINISTRATOR characters
    // MODERATOR, GAME MASTER, ADMINISTRATOR can see all
    if (pFriend && pFriend->GetName() && (!AccountMgr::IsPlayerAccount(security) || ((pFriend->GetTeam() == team || allowTwoSideWhoList) && (pFriend->GetSession()->GetSecurity() <= gmLevelInWhoList))) && pFriend->IsVisibleGloballyFor(player))
    {
        if (pFriend->isAFK())
            friendInfo.Status = FRIEND_STATUS_AFK;
        else if (pFriend->isDND())
            friendInfo.Status = FRIEND_STATUS_DND;
        else
        {
            friendInfo.Status = FRIEND_STATUS_ONLINE;

            if (pFriend->GetSession()->GetRecruiterId() == player->GetSession()->GetAccountId() || pFriend->GetSession()->GetAccountId() == player->GetSession()->GetRecruiterId())
                friendInfo.Status = FriendStatus(uint32(friendInfo.Status) | FRIEND_STATUS_RAF);
        }

        friendInfo.Area = pFriend->GetCurrentZoneID();
        friendInfo.Level = pFriend->getLevel();
        friendInfo.Class = pFriend->getClass();
    }
}

void SocialMgr::SendFriendStatus(Player* player, FriendsResult result, ObjectGuid const& friendGuid, bool broadcast)
{
    FriendInfo fi;
    GetFriendInfo(player, friendGuid, fi);

    WorldPackets::Social::FriendStatus friendStatus;
    friendStatus.VirtualRealmAddress = GetVirtualRealmAddress();
    friendStatus.Notes = fi.Note;
    friendStatus.ClassID = fi.Class;
    friendStatus.Status = fi.Status;
    friendStatus.Guid = friendGuid;
    friendStatus.WowAccountGuid = ObjectGuid::Create<HighGuid::WowAccount>(ObjectMgr::GetPlayerAccountIdByGUID(friendGuid));
    friendStatus.Level = fi.Level;
    friendStatus.AreaID = fi.Area;
    friendStatus.FriendResult = result;

    if (broadcast)
        BroadcastToFriendListers(player, friendStatus.Write());
    else
        player->SendDirectMessage(friendStatus.Write());
}

void SocialMgr::BroadcastToFriendListers(Player* player, WorldPacket const* packet)
{
    if (!player || m_socialMap.empty())
        return;

    for (auto const& friendPlayer : GetVisibleFriendsContaier(player))
        friendPlayer->SendDirectMessage(packet);
}

PlayerSocial* SocialMgr::LoadFromDB(PreparedQueryResult result, ObjectGuid const& guid)
{
    std::lock_guard<std::recursive_mutex> guard(m_social_lock);
    PlayerSocial *social = &m_socialMap[guid];
    social->SetPlayerGUID(guid);

    if (!result)
        return social;
    do
    {
        Field* fields = result->Fetch();

        ObjectGuid friendGuid = ObjectGuid::Create<HighGuid::Player>(fields[0].GetUInt64());
        uint8 flags = fields[1].GetUInt8();

        social->m_playerSocialMap.emplace(friendGuid, flags, fields[2].GetString());
    } while (result->NextRow());

    return social;
}

std::vector<Player*> SocialMgr::GetVisibleFriendsContaier(Player* player, bool online /*= false*/, uint32 lfgListActivityID /*= 0*/)
{
    auto data = std::vector<Player*>();
    if (!player || m_socialMap.empty())
        return data;

    std::lock_guard<std::recursive_mutex> guard(m_social_lock);
    auto team = player->GetTeam();
    auto security = player->GetSession()->GetSecurity();
    auto guid = player->GetGUID();
    auto gmLevelInWhoList = AccountTypes(sWorld->getIntConfig(CONFIG_GM_LEVEL_IN_WHO_LIST));
    auto allowTwoSideWhoList = sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_WHO_LIST);

    for (auto itr = m_socialMap.begin(); itr != m_socialMap.end(); ++itr)
    {
        auto itr2 = itr->second.m_playerSocialMap.get(guid);
        if (itr2 && itr2->second.Flags & SOCIAL_FLAG_FRIEND)
        {
            if (online && itr2->second.Status == FRIEND_STATUS_OFFLINE)
                continue;

            if (lfgListActivityID && itr2->second.LfgListActivityID != lfgListActivityID)
                continue;

            auto friendPlayer = ObjectAccessor::FindPlayer(itr->first);
            if (!friendPlayer)
                continue;

            if (friendPlayer->IsInWorld() && (!AccountMgr::IsPlayerAccount(friendPlayer->GetSession()->GetSecurity()) || ((friendPlayer->GetTeam() == team || allowTwoSideWhoList) && security <= gmLevelInWhoList)) && player->IsVisibleGloballyFor(friendPlayer))
                data.emplace_back(friendPlayer);
        }
    }

    return data;
}

GuidList SocialMgr::GetBNetFriendsGuids(uint32 /*lfgListActivityID*/)
{
    return GuidList();
}

GuidList SocialMgr::GetCharFriendsGuids(Player* player, uint32 lfgListActivityID)
{
    auto tmpList = GuidList();
    for (auto const& friendPlayer : GetVisibleFriendsContaier(player, true, lfgListActivityID))
        tmpList.emplace_back(friendPlayer->GetGUID());
    return tmpList;//_friendList;
}

GuidList SocialMgr::GetGuildMateGuids(uint32 lfgListActivityID)
{
    return _guildMateList;
}

bool SocialMgr::HasInFriendsList(Player * player, ObjectGuid guid)
{
    for (auto friendPlayer : GetVisibleFriendsContaier(player))
        if (friendPlayer->GetGUID() == guid)
            return true;

    return false;
}

void SocialMgr::RemovePlayerSocial(ObjectGuid const& guid)
{
    std::lock_guard<std::recursive_mutex> guard(m_social_lock);
    m_socialMap.erase(guid);
}
