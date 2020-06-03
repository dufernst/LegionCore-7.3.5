/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
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

#include "CalendarMgr.h"
#include <utility>
#include "DatabaseEnvFwd.h"
#include "Log.h"
#include "Player.h"
#include "GuildMgr.h"
#include "ObjectAccessor.h"
#include "Opcodes.h"
#include "CalendarPackets.h"
#include "WowTime.hpp"
#include "DatabaseEnv.h"
#include "Mail.h"

CalendarInvite::CalendarInvite(CalendarInvite const& calendarInvite, uint64 inviteId, uint64 eventId)
{
    _inviteId = inviteId;
    _eventId = eventId;
    _invitee = calendarInvite.GetInviteeGUID();
    _senderGUID = calendarInvite.GetSenderGUID();
    _responseTime = calendarInvite.GetResponseTime();
    _status = calendarInvite.GetStatus();
    _rank = calendarInvite.GetRank();
    _note = calendarInvite.GetNote();
}

CalendarInvite::CalendarInvite() : _inviteId(1), _eventId(0), _responseTime(0), _status(CALENDAR_STATUS_INVITED), _rank(CALENDAR_RANK_PLAYER), _note("")
{
}

CalendarInvite::CalendarInvite(uint64 inviteId, uint64 eventId, ObjectGuid invitee, ObjectGuid senderGUID, time_t responseTime, CalendarInviteStatus status, CalendarModerationRank rank, std::string note) :
    _inviteId(inviteId), _eventId(eventId), _invitee(invitee), _senderGUID(senderGUID), _responseTime(responseTime), _status(status), _rank(rank), _note(std::move(note))
{
}

CalendarInvite::~CalendarInvite()
{
    sCalendarMgr->FreeInviteId(_inviteId);
}

CalendarEvent::CalendarEvent(CalendarEvent const& calendarEvent, uint64 eventId)
{
    _eventId = eventId;
    _ownerGUID = calendarEvent.GetOwnerGUID();
    _eventGuildId = calendarEvent.GetGuildId();
    _eventType = calendarEvent.GetType();
    _textureId = calendarEvent.GetTextureId();
    _date = calendarEvent.GetDate();
    _flags = calendarEvent.GetFlags();
    _title = calendarEvent.GetTitle();
    _description = calendarEvent.GetDescription();
    _lockDate = calendarEvent.GetLockDate();
}

CalendarEvent::CalendarEvent(uint64 eventId, ObjectGuid ownerGUID, ObjectGuid::LowType guildId, CalendarEventType type, int32 textureId, time_t date, uint32 flags, std::string title, std::string description, time_t lockDate) :
    _eventId(eventId), _ownerGUID(ownerGUID), _eventGuildId(guildId), _eventType(type), _textureId(textureId), _date(date), _flags(flags), _title(std::move(title)), _description(std::move(description)), _lockDate(lockDate)
{
}

CalendarEvent::CalendarEvent() : _eventId(1), _eventGuildId(UI64LIT(0)), _eventType(CALENDAR_TYPE_OTHER), _textureId(-1), _date(0), _flags(0), _title(""), _description(""), _lockDate(0)
{
}

CalendarEvent::~CalendarEvent()
{
    sCalendarMgr->FreeEventId(_eventId);
}

CalendarMgr::CalendarMgr() : _maxEventId(0), _maxInviteId(0) { }

CalendarMgr::~CalendarMgr()
{
    for (auto itr : _events)
        delete itr;

    for (auto& itr : _invites)
        for (auto& itr2 : itr.second)
            delete itr2;
}

CalendarMgr* CalendarMgr::instance()
{
    static CalendarMgr instance;
    return &instance;
}

void CalendarMgr::LoadFromDB()
{
    uint32 count = 0;
    _maxEventId = 0;
    _maxInviteId = 0;

    //                                                       0        1      2      3            4          5          6     7      8
    if (QueryResult result = CharacterDatabase.Query("SELECT EventID, Owner, Title, Description, EventType, TextureID, Date, Flags, LockDate FROM calendar_events"))
        do
        {
            Field* fields = result->Fetch();

            uint64 eventID          = fields[0].GetUInt64();
            ObjectGuid ownerGUID    = ObjectGuid::Create<HighGuid::Player>(fields[1].GetUInt64());
            std::string title       = fields[2].GetString();
            std::string description = fields[3].GetString();
            auto type               = CalendarEventType(fields[4].GetUInt8());
            int32 textureID         = fields[5].GetInt32();
            uint32 date             = fields[6].GetUInt32();
            uint32 flags            = fields[7].GetUInt32();
            uint32 lockDate         = fields[8].GetUInt32();
            ObjectGuid::LowType guildID = UI64LIT(0);

            if (flags & CALENDAR_FLAG_GUILD_EVENT || flags & CALENDAR_FLAG_WITHOUT_INVITES)
                guildID = Player::GetGuildIdFromDB(ownerGUID);

            CalendarEvent* calendarEvent = new CalendarEvent(eventID, ownerGUID, guildID, type, textureID, time_t(date), flags, title, description, time_t(lockDate));
            _events.insert(calendarEvent);

            _maxEventId = std::max(_maxEventId, eventID);

            ++count;
        }
        while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_CALENDAR, ">> Loaded %u calendar events", count);
    count = 0;

    //                                                       0         1        2        3       4       5             6               7
    if (QueryResult result = CharacterDatabase.Query("SELECT InviteID, EventID, Invitee, Sender, Status, ResponseTime, ModerationRank, Note FROM calendar_invites"))
        do
        {
            Field* fields = result->Fetch();

            uint64 inviteId             = fields[0].GetUInt64();
            uint64 eventId              = fields[1].GetUInt64();
            ObjectGuid invitee          = ObjectGuid::Create<HighGuid::Player>(fields[2].GetUInt64());
            ObjectGuid senderGUID       = ObjectGuid::Create<HighGuid::Player>(fields[3].GetUInt64());
            auto status                 = CalendarInviteStatus(fields[4].GetUInt8());
            uint32 responseTime         = fields[5].GetUInt32();
            auto rank                   = CalendarModerationRank(fields[6].GetUInt8());
            std::string note            = fields[7].GetString();

            CalendarInvite* invite = new CalendarInvite(inviteId, eventId, invitee, senderGUID, time_t(responseTime), status, rank, note);
            _invites[eventId].push_back(invite);

            _maxInviteId = std::max(_maxInviteId, inviteId);

            ++count;
        }
        while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_CALENDAR, ">> Loaded %u calendar Invites", count);

    for (uint64 i = 1; i < _maxEventId; ++i)
        if (!GetEvent(i))
            _freeEventIds.push_back(i);

    for (uint64 i = 1; i < _maxInviteId; ++i)
        if (!GetInvite(i))
            _freeInviteIds.push_back(i);
}

void CalendarMgr::AddEvent(CalendarEvent* calendarEvent, CalendarSendEventType sendType)
{
    _events.insert(calendarEvent);
    UpdateEvent(calendarEvent);
    SendCalendarEvent(calendarEvent->GetOwnerGUID(), *calendarEvent, sendType);
}

void CalendarMgr::AddInvite(CalendarEvent* calendarEvent, CalendarInvite* invite)
{
    SQLTransaction dummy;
    AddInvite(calendarEvent, invite, dummy);
}

void CalendarMgr::AddInvite(CalendarEvent* calendarEvent, CalendarInvite* invite, SQLTransaction& trans)
{
    if (!calendarEvent->IsGuildAnnouncement() && calendarEvent->GetOwnerGUID() != invite->GetInviteeGUID())
        SendCalendarEventInvite(*invite);

    if (!calendarEvent->IsGuildEvent() || invite->GetInviteeGUID() == calendarEvent->GetOwnerGUID())
        SendCalendarEventInviteAlert(*calendarEvent, *invite);

    if (!calendarEvent->IsGuildAnnouncement())
    {
        _invites[invite->GetEventId()].push_back(invite);
        UpdateInvite(invite, trans);
    }
}

void CalendarMgr::RemoveEvent(uint64 eventId, ObjectGuid remover)
{
    CalendarEvent* calendarEvent = GetEvent(eventId);

    if (!calendarEvent)
    {
        SendCalendarCommandResult(remover, CALENDAR_ERROR_EVENT_INVALID);
        return;
    }

    SendCalendarEventRemovedAlert(*calendarEvent);

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    PreparedStatement* stmt;
    MailDraft mail(calendarEvent->BuildCalendarMailSubject(remover), calendarEvent->BuildCalendarMailBody());

    CalendarInviteStore& eventInvites = _invites[eventId];
    for (auto invite : eventInvites)
    {
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CALENDAR_INVITE);
        stmt->setUInt64(0, invite->GetInviteId());
        trans->Append(stmt);

        if (!remover.IsEmpty() && invite->GetInviteeGUID() != remover)
            mail.SendMailTo(trans, MailReceiver(invite->GetInviteeGUID().GetCounter()), calendarEvent, MAIL_CHECK_MASK_COPIED);

        delete invite;
    }

    _invites.erase(eventId);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CALENDAR_EVENT);
    stmt->setUInt64(0, eventId);
    trans->Append(stmt);
    CharacterDatabase.CommitTransaction(trans);

    delete calendarEvent;
    _events.erase(calendarEvent);
}

void CalendarMgr::RemoveInvite(uint64 inviteId, uint64 eventId, ObjectGuid /*remover*/)
{
    CalendarEvent* calendarEvent = GetEvent(eventId);

    if (!calendarEvent)
        return;

    auto itr = _invites[eventId].begin();
    for (; itr != _invites[eventId].end(); ++itr)
        if ((*itr)->GetInviteId() == inviteId)
            break;

    if (itr == _invites[eventId].end())
        return;

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CALENDAR_INVITE);
    stmt->setUInt64(0, (*itr)->GetInviteId());
    trans->Append(stmt);
    CharacterDatabase.CommitTransaction(trans);

    if (!calendarEvent->IsGuildEvent())
        SendCalendarEventInviteRemoveAlert((*itr)->GetInviteeGUID(), *calendarEvent, CALENDAR_STATUS_REMOVED);

    SendCalendarEventInviteRemove(*calendarEvent, **itr, calendarEvent->GetFlags());

    delete *itr;
    _invites[eventId].erase(itr);
}

void CalendarMgr::UpdateEvent(CalendarEvent* calendarEvent)
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_CALENDAR_EVENT);
    stmt->setUInt64(0, calendarEvent->GetEventId());
    stmt->setUInt64(1, calendarEvent->GetOwnerGUID().GetCounter());
    stmt->setString(2, calendarEvent->GetTitle());
    stmt->setString(3, calendarEvent->GetDescription());
    stmt->setUInt8(4, calendarEvent->GetType());
    stmt->setInt32(5, calendarEvent->GetTextureId());
    stmt->setUInt32(6, uint32(calendarEvent->GetDate()));
    stmt->setUInt32(7, calendarEvent->GetFlags());
    stmt->setUInt32(8, uint32(calendarEvent->GetLockDate()));
    CharacterDatabase.Execute(stmt);
}

void CalendarMgr::UpdateInvite(CalendarInvite* invite)
{
    SQLTransaction dummy;
    UpdateInvite(invite, dummy);
}

void CalendarMgr::UpdateInvite(CalendarInvite* invite, SQLTransaction& trans)
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_CALENDAR_INVITE);
    stmt->setUInt64(0, invite->GetInviteId());
    stmt->setUInt64(1, invite->GetEventId());
    stmt->setUInt64(2, invite->GetInviteeGUID().GetCounter());
    stmt->setUInt64(3, invite->GetSenderGUID().GetCounter());
    stmt->setUInt8(4, invite->GetStatus());
    stmt->setUInt32(5, uint32(invite->GetResponseTime()));
    stmt->setUInt8(6, invite->GetRank());
    stmt->setString(7, invite->GetNote());
    CharacterDatabase.ExecuteOrAppend(trans, stmt);
}

void CalendarMgr::RemoveAllPlayerEventsAndInvites(ObjectGuid guid)
{
    for (auto itr : _events)
        if (itr->GetOwnerGUID() == guid)
            RemoveEvent(itr->GetEventId(), ObjectGuid::Empty);

    CalendarInviteStore playerInvites = GetPlayerInvites(guid);
    for (CalendarInviteStore::const_iterator itr = playerInvites.begin(); itr != playerInvites.end(); ++itr)
        RemoveInvite((*itr)->GetInviteId(), (*itr)->GetEventId(), guid);
}

void CalendarMgr::RemovePlayerGuildEventsAndSignups(ObjectGuid guid, ObjectGuid::LowType guildId)
{
    for (auto itr : _events)
        if (itr->GetOwnerGUID() == guid && (itr->IsGuildEvent() || itr->IsGuildAnnouncement()))
            RemoveEvent(itr->GetEventId(), guid);

    CalendarInviteStore playerInvites = GetPlayerInvites(guid);
    for (CalendarInviteStore::const_iterator itr = playerInvites.begin(); itr != playerInvites.end(); ++itr)
        if (CalendarEvent* calendarEvent = GetEvent((*itr)->GetEventId()))
            if (calendarEvent->IsGuildEvent() && calendarEvent->GetGuildId() == guildId)
                RemoveInvite((*itr)->GetInviteId(), (*itr)->GetEventId(), guid);
}

CalendarEvent* CalendarMgr::GetEvent(uint64 eventId) const
{
    for (auto itr : _events)
        if (itr->GetEventId() == eventId)
            return itr;

    return nullptr;
}

CalendarInvite* CalendarMgr::GetInvite(uint64 inviteId) const
{
    for (const auto& itr : _invites)
        for (auto itr2 : itr.second)
            if (itr2->GetInviteId() == inviteId)
                return itr2;

    return nullptr;
}

void CalendarMgr::FreeEventId(uint64 id)
{
    if (id == _maxEventId)
        --_maxEventId;
    else
        _freeEventIds.push_back(id);
}

uint64 CalendarMgr::GetFreeEventId()
{
    if (_freeEventIds.empty())
        return ++_maxEventId;

    uint64 eventId = _freeEventIds.front();
    _freeEventIds.pop_front();

    return eventId;
}

void CalendarMgr::FreeInviteId(uint64 id)
{
    if (id == _maxInviteId)
        --_maxInviteId;
    else
        _freeInviteIds.push_back(id);
}

uint64 CalendarMgr::GetFreeInviteId()
{
    if (_freeInviteIds.empty())
        return ++_maxInviteId;

    uint64 inviteId = _freeInviteIds.front();
    _freeInviteIds.pop_front();
    return inviteId;
}

CalendarEventStore CalendarMgr::GetPlayerEvents(ObjectGuid guid)
{
    CalendarEventStore events;

    for (CalendarEventInviteStore::const_iterator itr = _invites.begin(); itr != _invites.end(); ++itr)
        for (auto itr2 = itr->second.begin(); itr2 != itr->second.end(); ++itr2)
            if ((*itr2)->GetInviteeGUID() == guid)
                if (CalendarEvent* event = GetEvent(itr->first))
                    events.insert(event);

    if (Player* player = ObjectAccessor::FindPlayer(guid))
        for (auto itr : _events)
            if (itr->GetGuildId() == player->GetGuildId())
                events.insert(itr);

    return events;
}

CalendarInviteStore const& CalendarMgr::GetEventInvites(uint64 eventId)
{
    return _invites[eventId];
}

CalendarInviteStore CalendarMgr::GetPlayerInvites(ObjectGuid guid)
{
    CalendarInviteStore invites;

    for (CalendarEventInviteStore::const_iterator itr = _invites.begin(); itr != _invites.end(); ++itr)
        for (auto itr2 : itr->second)
            if (itr2->GetInviteeGUID() == guid)
                invites.push_back(itr2);

    return invites;
}

uint32 CalendarMgr::GetPlayerNumPending(ObjectGuid guid)
{
    CalendarInviteStore const& invites = GetPlayerInvites(guid);

    uint32 pendingNum = 0;
    for (auto itr : invites)
    {
        switch (itr->GetStatus())
        {
            case CALENDAR_STATUS_INVITED:
            case CALENDAR_STATUS_TENTATIVE:
            case CALENDAR_STATUS_NOT_SIGNED_UP:
                ++pendingNum;
                break;
            default:
                break;
        }
    }

    return pendingNum;
}

std::string CalendarEvent::BuildCalendarMailSubject(ObjectGuid remover) const
{
    std::ostringstream strm;
    strm << remover << ':' << _title;
    return strm.str();
}

std::string CalendarEvent::BuildCalendarMailBody() const
{
    WorldPacket data;
    uint32 time;
    std::ostringstream strm;

    data << MS::Utilities::WowTime::Encode(_date);
    data >> time;
    strm << time;
    return strm.str();
}

void CalendarMgr::SendCalendarEventInvite(CalendarInvite const& invite)
{
    CalendarEvent* calendarEvent = GetEvent(invite.GetEventId());

    ObjectGuid invitee = invite.GetInviteeGUID();
    Player* player = ObjectAccessor::FindPlayer(invitee);

    WorldPackets::Calendar::SCalendarEventInvite packet;
    packet.EventID = calendarEvent ? calendarEvent->GetEventId() : 0;
    packet.InviteGuid = invitee;
    packet.InviteID = calendarEvent ? invite.GetInviteId() : 0;
    packet.Level = player ? player->getLevel() : Player::GetLevelFromDB(invitee);
    packet.ResponseTime = invite.GetResponseTime();
    packet.Status = invite.GetStatus();
    packet.Type = calendarEvent ? calendarEvent->IsGuildEvent() : 0;
    packet.ClearPending = calendarEvent ? !calendarEvent->IsGuildEvent() : true;

    if (!calendarEvent)
    {
        if (Player* playerSender = ObjectAccessor::FindPlayer(invite.GetSenderGUID()))
            playerSender->SendDirectMessage(packet.Write());
    }
    else
    {
        if (calendarEvent->GetOwnerGUID() != invite.GetInviteeGUID())
            SendPacketToAllEventRelatives(packet.Write(), *calendarEvent);
    }
}

void CalendarMgr::SendCalendarEventUpdateAlert(CalendarEvent const& calendarEvent, time_t originalDate)
{
    WorldPackets::Calendar::CalendarEventUpdatedAlert packet;
    packet.ClearPending = true;
    packet.Date = calendarEvent.GetDate();
    packet.Description = calendarEvent.GetDescription();
    packet.EventID = calendarEvent.GetEventId();
    packet.EventName = calendarEvent.GetTitle();
    packet.EventType = calendarEvent.GetType();
    packet.Flags = calendarEvent.GetFlags();
    packet.LockDate = calendarEvent.GetLockDate();
    packet.OriginalDate = originalDate;
    packet.TextureID = calendarEvent.GetTextureId();
    SendPacketToAllEventRelatives(packet.Write(), calendarEvent);
}

void CalendarMgr::SendCalendarEventStatus(CalendarEvent const& calendarEvent, CalendarInvite const& invite)
{
    WorldPackets::Calendar::CalendarEventInviteStatus packet;
    packet.ClearPending = true;
    packet.Date = calendarEvent.GetDate();
    packet.EventID = calendarEvent.GetEventId();
    packet.Flags = calendarEvent.GetFlags();
    packet.InviteGuid = invite.GetInviteeGUID();
    packet.ResponseTime = invite.GetResponseTime();
    packet.Status = invite.GetStatus();
    SendPacketToAllEventRelatives(packet.Write(), calendarEvent);
}

void CalendarMgr::SendCalendarEventRemovedAlert(CalendarEvent const& calendarEvent)
{
    WorldPackets::Calendar::CalendarEventRemovedAlert packet;
    packet.ClearPending = true;
    packet.Date = calendarEvent.GetDate();
    packet.EventID = calendarEvent.GetEventId();
    SendPacketToAllEventRelatives(packet.Write(), calendarEvent);
}

void CalendarMgr::SendCalendarEventInviteRemove(CalendarEvent const& calendarEvent, CalendarInvite const& invite, uint32 flags)
{
    WorldPackets::Calendar::CalendarEventInviteRemoved packet;
    packet.ClearPending = true;
    packet.EventID = calendarEvent.GetEventId();
    packet.Flags = flags;
    packet.InviteGuid = invite.GetInviteeGUID();
    SendPacketToAllEventRelatives(packet.Write(), calendarEvent);
}

void CalendarMgr::SendCalendarEventModeratorStatusAlert(CalendarEvent const& calendarEvent, CalendarInvite const& invite)
{
    WorldPackets::Calendar::CalendarEventInviteModeratorStatus packet;
    packet.ClearPending = true;
    packet.EventID = calendarEvent.GetEventId();
    packet.InviteGuid = invite.GetInviteeGUID();
    packet.Status = invite.GetStatus();
    SendPacketToAllEventRelatives(packet.Write(), calendarEvent);
}

void CalendarMgr::SendCalendarEventInviteAlert(CalendarEvent const& calendarEvent, CalendarInvite const& invite)
{
    WorldPackets::Calendar::CalendarEventInviteAlert packet;
    packet.Date = calendarEvent.GetDate();
    packet.EventID = calendarEvent.GetEventId();
    packet.EventName = calendarEvent.GetTitle();
    packet.EventType = calendarEvent.GetType();
    packet.Flags = calendarEvent.GetFlags();
    packet.InviteID = invite.GetInviteId();
    packet.InvitedByGuid = invite.GetSenderGUID();
    packet.ModeratorStatus = invite.GetRank();
    packet.OwnerGuid = calendarEvent.GetOwnerGUID();
    packet.Status = invite.GetStatus();
    packet.TextureID = calendarEvent.GetTextureId();

    Guild* guild = sGuildMgr->GetGuildById(calendarEvent.GetGuildId());
    packet.EventGuildID = guild ? guild->GetGUID() : ObjectGuid::Empty;

    if (calendarEvent.IsGuildEvent() || calendarEvent.IsGuildAnnouncement())
    {
        if (guild)
            guild->BroadcastPacket(packet.Write());
    }
    else
        if (Player* player = ObjectAccessor::FindPlayer(invite.GetInviteeGUID()))
            player->SendDirectMessage(packet.Write());
}

void CalendarMgr::SendCalendarEvent(ObjectGuid guid, CalendarEvent const& calendarEvent, CalendarSendEventType sendType)
{
    Player* player = ObjectAccessor::FindPlayer(guid);
    if (!player)
        return;

    CalendarInviteStore const& eventInviteeList = _invites[calendarEvent.GetEventId()];

    WorldPackets::Calendar::CalendarSendEvent packet;
    packet.Date = calendarEvent.GetDate();
    packet.Description = calendarEvent.GetDescription();
    packet.EventID = calendarEvent.GetEventId();
    packet.EventName = calendarEvent.GetTitle();
    packet.EventType = sendType;
    packet.Flags = calendarEvent.GetFlags();
    packet.GetEventType = calendarEvent.GetType();
    packet.LockDate = calendarEvent.GetLockDate();
    packet.OwnerGuid = calendarEvent.GetOwnerGUID();
    packet.TextureID = calendarEvent.GetTextureId();

    if (Guild* guild = sGuildMgr->GetGuildById(calendarEvent.GetGuildId()))
        packet.EventGuildID = guild->GetGUID();

    for (auto const& calendarInvite : eventInviteeList)
    {
        ObjectGuid inviteeGuid = calendarInvite->GetInviteeGUID();
        Player* invitee = ObjectAccessor::FindPlayer(inviteeGuid);

        uint8 inviteeLevel = invitee ? invitee->getLevel() : Player::GetLevelFromDB(inviteeGuid);
        ObjectGuid::LowType inviteeGuildId = invitee ? invitee->GetGuildId() : Player::GetGuildIdFromDB(inviteeGuid);

        WorldPackets::Calendar::CalendarEventInviteInfo inviteInfo;
        inviteInfo.Guid = inviteeGuid;
        inviteInfo.Level = inviteeLevel;
        inviteInfo.Status = calendarInvite->GetStatus();
        inviteInfo.Moderator = calendarInvite->GetRank();
        inviteInfo.InviteType = calendarEvent.IsGuildEvent() && calendarEvent.GetGuildId() == inviteeGuildId;
        inviteInfo.InviteID = calendarInvite->GetInviteId();
        inviteInfo.ResponseTime = calendarInvite->GetResponseTime();
        inviteInfo.Notes = calendarInvite->GetNote();

        packet.Invites.push_back(inviteInfo);
    }

    player->SendDirectMessage(packet.Write());
}

void CalendarMgr::SendCalendarEventInviteRemoveAlert(ObjectGuid guid, CalendarEvent const& calendarEvent, CalendarInviteStatus status)
{
    if (Player* player = ObjectAccessor::FindPlayer(guid))
    {
        WorldPackets::Calendar::CalendarEventInviteRemovedAlert packet;
        packet.Date = calendarEvent.GetDate();
        packet.EventID = calendarEvent.GetEventId();
        packet.Flags = calendarEvent.GetFlags();
        packet.Status = status;
        player->SendDirectMessage(packet.Write());
    }
}

void CalendarMgr::SendCalendarClearPendingAction(ObjectGuid guid)
{
    if (Player* player = ObjectAccessor::FindPlayer(guid))
        player->SendDirectMessage(WorldPackets::Calendar::CalendarClearPendingAction().Write());
}

void CalendarMgr::SendCalendarCommandResult(ObjectGuid guid, CalendarError err, char const* param /*= NULL*/)
{
    if (Player* player = ObjectAccessor::FindPlayer(guid))
    {
        WorldPackets::Calendar::CalendarCommandResult packet;
        packet.Command = 1;
        packet.Result = err;

        switch (err)
        {
            case CALENDAR_ERROR_OTHER_INVITES_EXCEEDED:
            case CALENDAR_ERROR_ALREADY_INVITED_TO_EVENT_S:
            case CALENDAR_ERROR_IGNORING_YOU_S:
                packet.Name = param;
                break;
            default:
                break;
        }

        player->SendDirectMessage(packet.Write());
    }
}

void CalendarMgr::SendPacketToAllEventRelatives(WorldPacket const* packet, CalendarEvent const& calendarEvent)
{
    if (calendarEvent.IsGuildEvent() || calendarEvent.IsGuildAnnouncement())
        if (Guild* guild = sGuildMgr->GetGuildById(calendarEvent.GetGuildId()))
            guild->BroadcastPacket(packet);

    CalendarInviteStore invites = _invites[calendarEvent.GetEventId()];
    for (auto& itr : invites)
        if (Player* player = ObjectAccessor::FindPlayer(itr->GetInviteeGUID()))
            if (!calendarEvent.IsGuildEvent() || (calendarEvent.IsGuildEvent() && player->GetGuildId() != calendarEvent.GetGuildId()))
                player->SendDirectMessage(packet);
}
