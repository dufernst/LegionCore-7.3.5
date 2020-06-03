/*
 * Copyright (C) 2012 TrinityCore <http://www.trinitycore.org/>
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

#include "WorldSession.h"
#include "Player.h"
#include "Opcodes.h"
#include "ReferAFriendPackets.h"

void WorldSession::HandleGrantLevel(WorldPackets::ReferAFriend::GrantLevel& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    Player* target = ObjectAccessor::GetObjectInWorld(packet.Target, player);

    // check cheating
    uint8 levels = player->GetGrantableLevels();
    uint8 error = 0;
    if (!target)
        error = ERR_REFER_A_FRIEND_NO_TARGET;
    else if (levels == 0)
        error = ERR_REFER_A_FRIEND_INSUFFICIENT_GRANTABLE_LEVELS;
    else if (GetRecruiterId() != target->GetSession()->GetAccountId())
        error = ERR_REFER_A_FRIEND_NOT_REFERRED_BY;
    else if (target->GetTeamId() != player->GetTeamId())
        error = ERR_REFER_A_FRIEND_DIFFERENT_FACTION;
    else if (target->getLevel() >= player->getLevel())
        error = ERR_REFER_A_FRIEND_TARGET_TOO_HIGH;
    else if (target->getLevel() >= sWorld->getIntConfig(CONFIG_MAX_RECRUIT_A_FRIEND_BONUS_PLAYER_LEVEL))
        error = ERR_REFER_A_FRIEND_GRANT_LEVEL_MAX_I;
    else if (target->GetGroup() != player->GetGroup())
        error = ERR_REFER_A_FRIEND_NOT_IN_GROUP;

    if (error) 
    {
        WorldPackets::ReferAFriend::ReferAFriendFailure failure;
        if (error == ERR_REFER_A_FRIEND_NOT_IN_GROUP)
            failure.Str =  target->GetName();
        failure.Reason = error;
        SendPacket(failure.Write());
        return;
    }

    WorldPackets::ReferAFriend::ProposeLevelGrant grant;
    grant.Sender = player->GetGUID();
    target->SendDirectMessage(grant.Write());
}

void WorldSession::HandleAcceptGrantLevel(WorldPackets::ReferAFriend::AcceptLevelGrant& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    Player* other = ObjectAccessor::GetObjectInWorld(packet.Granter, player);
    if (!(other && other->GetSession()))
        return;

    if (GetAccountId() != other->GetSession()->GetRecruiterId())
        return;

    if (other->GetGrantableLevels())
        other->SetGrantableLevels(other->GetGrantableLevels() - 1);
    else
        return;

    player->GiveLevel(player->getLevel() + 1);
}
