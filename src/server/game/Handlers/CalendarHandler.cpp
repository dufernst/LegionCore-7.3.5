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

#include "CalendarPackets.h"
#include "GuildMgr.h"
#include "InstanceSaveMgr.h"
#include "DatabaseEnv.h"
#include "SocialMgr.h"

void WorldSession::HandleCalendarGetCalendar(WorldPackets::Calendar::CalendarGetCalendar& /*packet*/)
{
    ObjectGuid guid = _player->GetGUID();
    time_t currTime = time(nullptr);

    WorldPackets::Calendar::CalendarSendCalendar packet;
    packet.ServerTime = currTime;

    CalendarInviteStore playerInvites = sCalendarMgr->GetPlayerInvites(guid);
    for (auto const& invite : playerInvites)
    {
        WorldPackets::Calendar::CalendarSendCalendarInviteInfo inviteInfo;
        inviteInfo.EventID = invite->GetEventId();
        inviteInfo.InviteID = invite->GetInviteId();
        inviteInfo.InviterGuid = invite->GetSenderGUID();
        inviteInfo.Status = invite->GetStatus();
        inviteInfo.Moderator = invite->GetRank();
        if (CalendarEvent* calendarEvent = sCalendarMgr->GetEvent(invite->GetEventId()))
            inviteInfo.InviteType = calendarEvent->IsGuildEvent() && calendarEvent->GetGuildId() == _player->GetGuildId();

        packet.Invites.push_back(inviteInfo);
    }

    CalendarEventStore playerEvents = sCalendarMgr->GetPlayerEvents(guid);
    for (auto const& event : playerEvents)
    {
        WorldPackets::Calendar::CalendarSendCalendarEventInfo eventInfo;
        eventInfo.EventID = event->GetEventId();
        eventInfo.Date = event->GetDate();
        if (Guild* guild = sGuildMgr->GetGuildById(event->GetGuildId()))
            eventInfo.EventGuildID = guild->GetGUID();
        eventInfo.EventName = event->GetTitle();
        eventInfo.EventType = event->GetType();
        eventInfo.Flags = event->GetFlags();
        eventInfo.OwnerGuid = event->GetOwnerGUID();
        eventInfo.TextureID = event->GetTextureId();

        packet.Events.push_back(eventInfo);
    }

    for (uint8 i = 0; i < MAX_BOUND; ++i)
    {
        Player::BoundInstancesMap boundInstances = _player->GetBoundInstances(Difficulty(i));
        for (auto const& boundInstance : boundInstances)
        {
            if (InstanceSave* save = boundInstance.second.save)
            {
                WorldPackets::Calendar::CalendarSendCalendarRaidLockoutInfo lockoutInfo;
                lockoutInfo.MapID = save->GetMapId();
                lockoutInfo.DifficultyID = save->GetDifficultyID();
                lockoutInfo.ExpireTime = save->GetResetTime() - currTime;
                lockoutInfo.InstanceID = save->GetInstanceId();
                packet.RaidLockouts.push_back(lockoutInfo);
            }
        }
    }

    SendPacket(packet.Write());
}

void WorldSession::HandleCalendarGetEvent(WorldPackets::Calendar::CalendarGetEvent& packet)
{
    if (CalendarEvent* calendarEvent = sCalendarMgr->GetEvent(packet.EventID))
        sCalendarMgr->SendCalendarEvent(_player->GetGUID(), *calendarEvent, CALENDAR_SENDTYPE_GET);
    else
        sCalendarMgr->SendCalendarCommandResult(_player->GetGUID(), CALENDAR_ERROR_EVENT_INVALID);
}

void WorldSession::HandleCalendarGuildFilter(WorldPackets::Calendar::CalendarGuildFilter& packet)
{
    if (Guild* guild = sGuildMgr->GetGuildById(_player->GetGuildId()))
        guild->MassInviteToEvent(this, packet.MinLevel, packet.MaxLevel, packet.MaxRankOrder);
}

void WorldSession::HandleCalendarAddEvent(WorldPackets::Calendar::CalendarAddEvent& packet)
{
    ObjectGuid guid = _player->GetGUID();

    if (packet.EventInfo.Time < (time(nullptr) - time_t(86400L)))
        return;

    // Strip invisible characters for non-addon messages
    if (sWorld->getBoolConfig(CONFIG_CHAT_FAKE_MESSAGE_PREVENTING))
    {
        stripLineInvisibleChars(packet.EventInfo.Title);
        stripLineInvisibleChars(packet.EventInfo.Description);

        if (strchr(packet.EventInfo.Title.c_str(), '|') || strchr(packet.EventInfo.Description.c_str(), '|'))
        {
            if (sWorld->getIntConfig(CONFIG_CHAT_STRICT_LINK_CHECKING_KICK))
                KickPlayer();
            return;
        }
    }

    CalendarEvent* calendarEvent = new CalendarEvent(sCalendarMgr->GetFreeEventId(), guid, UI64LIT(0), CalendarEventType(packet.EventInfo.EventType), packet.EventInfo.TextureID,
        packet.EventInfo.Time, packet.EventInfo.Flags, packet.EventInfo.Title, packet.EventInfo.Description, time_t(0));

    if (calendarEvent->IsGuildEvent() || calendarEvent->IsGuildAnnouncement())
        if (Player* creator = ObjectAccessor::FindPlayer(guid))
            calendarEvent->SetGuildId(creator->GetGuildId());

    if (calendarEvent->IsGuildAnnouncement())
    {
        CalendarInvite invite(0, calendarEvent->GetEventId(), ObjectGuid::Empty, guid, CALENDAR_DEFAULT_RESPONSE_TIME, CALENDAR_STATUS_NOT_SIGNED_UP, CALENDAR_RANK_PLAYER, "");
        sCalendarMgr->AddInvite(calendarEvent, &invite);
    }
    else
    {
        SQLTransaction trans;
        if (packet.EventInfo.Invites.size() > 1)
            trans = CharacterDatabase.BeginTransaction();

        for (auto i = 0; i < packet.EventInfo.Invites.size(); ++i)
        {
            CalendarInvite* invite = new CalendarInvite(sCalendarMgr->GetFreeInviteId(), calendarEvent->GetEventId(), packet.EventInfo.Invites[i].Guid,
                guid, CALENDAR_DEFAULT_RESPONSE_TIME, CalendarInviteStatus(packet.EventInfo.Invites[i].Status),
                CalendarModerationRank(packet.EventInfo.Invites[i].Moderator), "");
            sCalendarMgr->AddInvite(calendarEvent, invite, trans);
        }

        if (packet.EventInfo.Invites.size() > 1)
            CharacterDatabase.CommitTransaction(trans);
    }

    sCalendarMgr->AddEvent(calendarEvent, CALENDAR_SENDTYPE_ADD);
}

void WorldSession::HandleCalendarUpdateEvent(WorldPackets::Calendar::CalendarUpdateEvent& packet)
{
    auto oldEventTime = time_t(0);

    if (packet.EventInfo.Time < (time(nullptr) - time_t(86400L)))
        return;

    // Strip invisible characters for non-addon messages
    if (sWorld->getBoolConfig(CONFIG_CHAT_FAKE_MESSAGE_PREVENTING))
    {
        stripLineInvisibleChars(packet.EventInfo.Title);
        stripLineInvisibleChars(packet.EventInfo.Description);

        if (strchr(packet.EventInfo.Title.c_str(), '|') || strchr(packet.EventInfo.Description.c_str(), '|'))
        {
            if (sWorld->getIntConfig(CONFIG_CHAT_STRICT_LINK_CHECKING_KICK))
                KickPlayer();
            return;
        }
    }

    if (CalendarEvent* calendarEvent = sCalendarMgr->GetEvent(packet.EventInfo.EventID))
    {
        oldEventTime = calendarEvent->GetDate();

        calendarEvent->SetType(CalendarEventType(packet.EventInfo.EventType));
        calendarEvent->SetFlags(packet.EventInfo.Flags);
        calendarEvent->SetDate(packet.EventInfo.Time);
        calendarEvent->SetTextureId(packet.EventInfo.TextureID);
        calendarEvent->SetTitle(packet.EventInfo.Title);
        calendarEvent->SetDescription(packet.EventInfo.Description);

        sCalendarMgr->UpdateEvent(calendarEvent);
        sCalendarMgr->SendCalendarEventUpdateAlert(*calendarEvent, oldEventTime);
    }
    else
        sCalendarMgr->SendCalendarCommandResult(_player->GetGUID(), CALENDAR_ERROR_EVENT_INVALID);
}

void WorldSession::HandleCalendarRemoveEvent(WorldPackets::Calendar::CalendarRemoveEvent& packet)
{
    sCalendarMgr->RemoveEvent(packet.EventID, _player->GetGUID());
}

void WorldSession::HandleCalendarCopyEvent(WorldPackets::Calendar::CalendarCopyEvent& packet)
{
    ObjectGuid guid = _player->GetGUID();

    if (packet.Date < (time(nullptr) - time_t(86400L)))
        return;

    if (CalendarEvent* oldEvent = sCalendarMgr->GetEvent(packet.EventID))
    {
        auto newEvent = new CalendarEvent(*oldEvent, sCalendarMgr->GetFreeEventId());
        newEvent->SetDate(packet.Date);
        sCalendarMgr->AddEvent(newEvent, CALENDAR_SENDTYPE_COPY);

        CalendarInviteStore invites = sCalendarMgr->GetEventInvites(packet.EventID);
        SQLTransaction trans;
        if (invites.size() > 1)
            trans = CharacterDatabase.BeginTransaction();

        for (CalendarInviteStore::const_iterator itr = invites.begin(); itr != invites.end(); ++itr)
            sCalendarMgr->AddInvite(newEvent, new CalendarInvite(**itr, sCalendarMgr->GetFreeInviteId(), newEvent->GetEventId()), trans);

        if (invites.size() > 1)
            CharacterDatabase.CommitTransaction(trans);
    }
    else
        sCalendarMgr->SendCalendarCommandResult(guid, CALENDAR_ERROR_EVENT_INVALID);
}

void WorldSession::HandleCalendarEventInvite(WorldPackets::Calendar::CalendarEventInvite& packet)
{
    ObjectGuid playerGuid = _player->GetGUID();

    ObjectGuid inviteeGuid;
    uint32 inviteeTeam = 0;
    ObjectGuid::LowType inviteeGuildId = UI64LIT(0);

    // Strip invisible characters for non-addon messages
    if (sWorld->getBoolConfig(CONFIG_CHAT_FAKE_MESSAGE_PREVENTING))
    {
        stripLineInvisibleChars(packet.Name);

        if (strchr(packet.Name.c_str(), '|'))
        {

            if (sWorld->getIntConfig(CONFIG_CHAT_STRICT_LINK_CHECKING_KICK))
                KickPlayer();
            
            return;
        }
    }

    if (Player* player = ObjectAccessor::FindPlayerByName(packet.Name))
    {
        inviteeGuid = player->GetGUID();
        inviteeTeam = player->GetTeam();
        inviteeGuildId = player->GetGuildId();
    }
    else
    {
        if (const CharacterInfo* nameData = sWorld->GetCharacterInfo(packet.Name))
        {
            inviteeGuid = ObjectGuid::Create<HighGuid::Player>(nameData->Guid);
            inviteeTeam = Player::TeamForRace(nameData->Race);
            inviteeGuildId = nameData->GuildId;
        }
    }

    if (!inviteeGuid)
    {
        sCalendarMgr->SendCalendarCommandResult(playerGuid, CALENDAR_ERROR_PLAYER_NOT_FOUND);
        return;
    }

    if (_player->GetTeam() != inviteeTeam && !sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CALENDAR))
    {
        sCalendarMgr->SendCalendarCommandResult(playerGuid, CALENDAR_ERROR_NOT_ALLIED);
        return;
    }

    if (QueryResult result = CharacterDatabase.PQuery("SELECT flags FROM character_social WHERE guid = %u AND friend = %u", inviteeGuid.GetGUIDLow(), playerGuid.GetGUIDLow()))
    {
        Field* fields = result->Fetch();
        if (fields[0].GetUInt8() & SOCIAL_FLAG_IGNORED)
        {
            sCalendarMgr->SendCalendarCommandResult(playerGuid, CALENDAR_ERROR_IGNORING_YOU_S, packet.Name.c_str());
            return;
        }
    }

    if (!packet.Creating)
    {
        if (CalendarEvent* calendarEvent = sCalendarMgr->GetEvent(packet.EventID))
        {
            if (calendarEvent->IsGuildEvent() && calendarEvent->GetGuildId() == inviteeGuildId)
            {
                sCalendarMgr->SendCalendarCommandResult(playerGuid, CALENDAR_ERROR_NO_GUILD_INVITES);
                return;
            }

            CalendarInvite* invite = new CalendarInvite(sCalendarMgr->GetFreeInviteId(), packet.EventID, inviteeGuid, playerGuid, CALENDAR_DEFAULT_RESPONSE_TIME, CALENDAR_STATUS_INVITED, CALENDAR_RANK_PLAYER, "");
            sCalendarMgr->AddInvite(calendarEvent, invite);
        }
        else
            sCalendarMgr->SendCalendarCommandResult(playerGuid, CALENDAR_ERROR_EVENT_INVALID);
    }
    else
    {
        if (packet.IsSignUp && inviteeGuildId == _player->GetGuildId())
        {
            sCalendarMgr->SendCalendarCommandResult(playerGuid, CALENDAR_ERROR_NO_GUILD_INVITES);
            return;
        }

        CalendarInvite invite(sCalendarMgr->GetFreeInviteId(), 0L, inviteeGuid, playerGuid, CALENDAR_DEFAULT_RESPONSE_TIME, CALENDAR_STATUS_INVITED, CALENDAR_RANK_PLAYER, "");
        sCalendarMgr->SendCalendarEventInvite(invite);
    }
}

void WorldSession::HandleCalendarEventSignup(WorldPackets::Calendar::CalendarEventSignUp& packet)
{
    ObjectGuid guid = _player->GetGUID();

    if (CalendarEvent* calendarEvent = sCalendarMgr->GetEvent(packet.EventID))
    {
        if (calendarEvent->IsGuildEvent() && calendarEvent->GetGuildId() != _player->GetGuildId())
        {
            sCalendarMgr->SendCalendarCommandResult(guid, CALENDAR_ERROR_GUILD_PLAYER_NOT_IN_GUILD);
            return;
        }

        CalendarInviteStatus status = packet.Tentative ? CALENDAR_STATUS_TENTATIVE : CALENDAR_STATUS_SIGNED_UP;
        CalendarInvite* invite = new CalendarInvite(sCalendarMgr->GetFreeInviteId(), packet.EventID, guid, guid, time(nullptr), status, CALENDAR_RANK_PLAYER, "");
        sCalendarMgr->AddInvite(calendarEvent, invite);
        sCalendarMgr->SendCalendarClearPendingAction(guid);
    }
    else
        sCalendarMgr->SendCalendarCommandResult(guid, CALENDAR_ERROR_EVENT_INVALID);
}

void WorldSession::HandleCalendarEventRsvp(WorldPackets::Calendar::CalendarEventRSVP& packet)
{
    ObjectGuid guid = _player->GetGUID();

    if (CalendarEvent* calendarEvent = sCalendarMgr->GetEvent(packet.EventID))
    {
        if (packet.Status != CALENDAR_STATUS_REMOVED && calendarEvent->IsLocked())
        {
            sCalendarMgr->SendCalendarCommandResult(guid, CALENDAR_ERROR_EVENT_LOCKED);
            return;
        }

        if (CalendarInvite* invite = sCalendarMgr->GetInvite(packet.InviteID))
        {
            invite->SetStatus(CalendarInviteStatus(packet.Status));
            invite->SetResponseTime(time(nullptr));

            sCalendarMgr->UpdateInvite(invite);
            sCalendarMgr->SendCalendarEventStatus(*calendarEvent, *invite);
            sCalendarMgr->SendCalendarClearPendingAction(guid);
        }
        else
            sCalendarMgr->SendCalendarCommandResult(guid, CALENDAR_ERROR_NO_INVITE);
    }
    else
        sCalendarMgr->SendCalendarCommandResult(guid, CALENDAR_ERROR_EVENT_INVALID);
}

void WorldSession::HandleCalendarEventRemoveInvite(WorldPackets::Calendar::CalendarRemoveInvite& packet)
{
    ObjectGuid guid = _player->GetGUID();

    if (CalendarEvent* calendarEvent = sCalendarMgr->GetEvent(packet.EventID))
    {
        if (calendarEvent->GetOwnerGUID() == packet.Guid)
        {
            sCalendarMgr->SendCalendarCommandResult(guid, CALENDAR_ERROR_DELETE_CREATOR_FAILED);
            return;
        }

        sCalendarMgr->RemoveInvite(packet.InviteID, packet.EventID, guid);
    }
    else
        sCalendarMgr->SendCalendarCommandResult(guid, CALENDAR_ERROR_NO_INVITE);
}

void WorldSession::HandleCalendarEventStatus(WorldPackets::Calendar::CalendarEventStatus& packet)
{
    ObjectGuid guid = _player->GetGUID();

    if (CalendarEvent* calendarEvent = sCalendarMgr->GetEvent(packet.EventID))
    {
        if (CalendarInvite* invite = sCalendarMgr->GetInvite(packet.InviteID))
        {
            invite->SetStatus(static_cast<CalendarInviteStatus>(packet.Status));

            sCalendarMgr->UpdateInvite(invite);
            sCalendarMgr->SendCalendarEventStatus(*calendarEvent, *invite);
            sCalendarMgr->SendCalendarClearPendingAction(packet.Guid);
        }
        else
            sCalendarMgr->SendCalendarCommandResult(guid, CALENDAR_ERROR_NO_INVITE);
    }
    else
        sCalendarMgr->SendCalendarCommandResult(guid, CALENDAR_ERROR_EVENT_INVALID);
}

void WorldSession::HandleCalendarEventModeratorStatus(WorldPackets::Calendar::CalendarEventModeratorStatus& packet)
{
    ObjectGuid guid = _player->GetGUID();

    if (CalendarEvent* calendarEvent = sCalendarMgr->GetEvent(packet.EventID))
    {
        if (CalendarInvite* invite = sCalendarMgr->GetInvite(packet.InviteID))
        {
            invite->SetRank(CalendarModerationRank(packet.Status));
            sCalendarMgr->UpdateInvite(invite);
            sCalendarMgr->SendCalendarEventModeratorStatusAlert(*calendarEvent, *invite);
        }
        else
            sCalendarMgr->SendCalendarCommandResult(guid, CALENDAR_ERROR_NO_INVITE);
    }
    else
        sCalendarMgr->SendCalendarCommandResult(guid, CALENDAR_ERROR_EVENT_INVALID);
}

void WorldSession::HandleCalendarComplain(WorldPackets::Calendar::CalendarComplain& /*packet*/)
{ }

void WorldSession::HandleCalendarGetNumPending(WorldPackets::Calendar::CalendarGetNumPending& /*packet*/)
{
    SendPacket(WorldPackets::Calendar::CalendarSendNumPending(sCalendarMgr->GetPlayerNumPending(_player->GetGUID())).Write());
}
