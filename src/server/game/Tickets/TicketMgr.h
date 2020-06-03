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

#ifndef _TICKETMGR_H
#define _TICKETMGR_H

#include "ObjectMgr.h"

class ChatHandler;

// from blizzard lua
enum GMTicketSystemStatus
{
    GMTICKET_QUEUE_STATUS_DISABLED = -1,
    GMTICKET_QUEUE_STATUS_ENABLED = 1,
};

enum GMTicketStatus
{
    GMTICKET_STATUS_DB_ERROR                     = 0x00,
    GMTICKET_STATUS_HASTEXT                      = 0x06,
    GMTICKET_STATUS_DEFAULT                      = 0x0A,
};

enum GMTicketResponse
{
    GMTICKET_RESPONSE_ALREADY_EXIST               = 1,
    GMTICKET_RESPONSE_CREATE_SUCCESS              = 2,
    GMTICKET_RESPONSE_CREATE_ERROR                = 3,
    GMTICKET_RESPONSE_UPDATE_SUCCESS              = 4,
    GMTICKET_RESPONSE_UPDATE_ERROR                = 5,
    GMTICKET_RESPONSE_TICKET_DELETED              = 9,
    GMTICKET_RESPONSE_DB_ERROR                    = 10,
};

// from Blizzard LUA:
// GMTICKET_ASSIGNEDTOGM_STATUS_NOT_ASSIGNED = 0;    -- ticket is not currently assigned to a gm
// GMTICKET_ASSIGNEDTOGM_STATUS_ASSIGNED = 1;        -- ticket is assigned to a normal gm
// GMTICKET_ASSIGNEDTOGM_STATUS_ESCALATED = 2;        -- ticket is in the escalation queue
// 3 is a custom value and should never actually be sent
enum GMTicketEscalationStatus
{
    TICKET_UNASSIGNED                             = 0,
    TICKET_ASSIGNED                               = 1,
    TICKET_IN_ESCALATION_QUEUE                    = 2,
    TICKET_ESCALATED_ASSIGNED                     = 3,
};

// from blizzard lua
enum GMTicketOpenedByGMStatus
{
    GMTICKET_OPENEDBYGM_STATUS_NOT_OPENED = 0,      // ticket has never been opened by a gm
    GMTICKET_OPENEDBYGM_STATUS_OPENED = 1,          // ticket has been opened by a gm
};

enum LagReportType
{
    LAG_REPORT_TYPE_LOOT = 1,
    LAG_REPORT_TYPE_AUCTION_HOUSE = 2,
    LAG_REPORT_TYPE_MAIL = 3,
    LAG_REPORT_TYPE_CHAT = 4,
    LAG_REPORT_TYPE_MOVEMENT = 5,
    LAG_REPORT_TYPE_SPELL = 6
};

class GmTicket
{
public:
    GmTicket();
    explicit GmTicket(Player* player, WorldPackets::Ticket::SupportTicketSubmitBug& packet);
    ~GmTicket();

    bool IsClosed() const { return !_closedBy.IsEmpty(); }
    bool IsCompleted() const { return _completed; }
    bool IsFromPlayer(ObjectGuid const& guid) const { return guid == _playerGuid; }
    bool IsAssigned() const { return _assignedTo; }
    bool IsAssignedTo(ObjectGuid const& guid) const { return guid == _assignedTo; }
    bool IsAssignedNotTo(ObjectGuid guid) const { return IsAssigned() && !IsAssignedTo(guid); }

    uint32 GetId() const { return _id; }
    Player* GetPlayer() const { return ObjectAccessor::FindPlayer(_playerGuid); }
    std::string GetPlayerName() const { return _playerName; }
    std::string const& GetMessage() const { return _message; }
    std::string const& GetResponse() const { return _response; }
    Player* GetAssignedPlayer() const { return ObjectAccessor::FindPlayer(_assignedTo); }
    ObjectGuid GetAssignedToGUID() const { return _assignedTo; }
    std::string GetAssignedToName() const
    {
        std::string name;
        // save queries if ticket is not assigned
        if (_assignedTo)
            sObjectMgr->GetPlayerNameByGUID(_assignedTo, name);

        return name;
    }
    uint64 GetLastModifiedTime() const { return _lastModifiedTime; }
    GMTicketEscalationStatus GetEscalatedStatus() const { return _escalatedStatus; }

    void SetEscalatedStatus(GMTicketEscalationStatus escalatedStatus) { _escalatedStatus = escalatedStatus; }
    void SetAssignedTo(ObjectGuid const& guid, bool isAdmin)
    {
        _assignedTo = guid;
        if (isAdmin && _escalatedStatus == TICKET_IN_ESCALATION_QUEUE)
            _escalatedStatus = TICKET_ESCALATED_ASSIGNED;
        else if (_escalatedStatus == TICKET_UNASSIGNED)
            _escalatedStatus = TICKET_ASSIGNED;
    }
    void SetClosedBy(ObjectGuid const& value) { _closedBy = value; }
    void SetMessage(std::string const& message)
    {
        _message = message;
        _lastModifiedTime = uint64(time(nullptr));
    }
    void SetComment(std::string const& comment) { _comment = comment; }
    void SetViewed() { _viewed = true; }
    void SetUnassigned();
    void SetCompleted(bool complete) { _completed = complete; }

    void AppendResponse(std::string const& response) { _response += response; }

    bool LoadFromDB(Field* fields);
    void SaveToDB(SQLTransaction& trans) const;
    void DeleteFromDB();

    void TeleportTo(Player* player) const;
    std::string FormatMessageString(ChatHandler& handler, bool detailed = false) const;
    std::string FormatMessageString(ChatHandler& handler, const char* szClosedName, const char* szAssignedToName, const char* szUnassignedName, const char* szDeletedName) const;

private:
    uint32 _id;
    ObjectGuid _playerGuid;
    std::string _playerName;
    Position _pos;
    uint32 _mapId;
    std::string _message;
    uint64 _createTime;
    uint64 _lastModifiedTime;
    ObjectGuid _closedBy; // 0 = Open, -1 = Console, playerGuid = player abandoned ticket, other = GM who closed it.
    ObjectGuid _assignedTo;
    std::string _comment;
    bool _completed;
    GMTicketEscalationStatus _escalatedStatus;
    bool _viewed;
    std::string _response;
};
typedef std::map<uint32, GmTicket*> GmTicketList;

class TicketMgr
{
private:
    TicketMgr();
    ~TicketMgr();

public:
    static TicketMgr* instance()
    {
        static TicketMgr instance;
        return &instance;
    }

    void LoadTickets();
    void LoadSurveys();

    GmTicket* GetTicket(uint32 ticketId)
    {
        return Trinity::Containers::MapGetValuePtr(_ticketList, ticketId);
    }

    GmTicket* GetTicketByPlayer(ObjectGuid const& playerGuid)
    {
        for (GmTicketList::const_iterator itr = _ticketList.begin(); itr != _ticketList.end(); ++itr)
            if (itr->second && itr->second->IsFromPlayer(playerGuid) && !itr->second->IsClosed())
                return itr->second;

        return nullptr;
    }

    GmTicket* GetOldestOpenTicket()
    {
        for (GmTicketList::const_iterator itr = _ticketList.begin(); itr != _ticketList.end(); ++itr)
            if (itr->second && !itr->second->IsClosed() && !itr->second->IsCompleted())
                return itr->second;

        return nullptr;
    }

    void AddTicket(GmTicket* ticket);
    void CloseTicket(uint32 ticketId, ObjectGuid source);
    void RemoveTicket(uint32 ticketId);

    bool GetStatus() const { return _status; }
    void SetStatus(bool status) { _status = status; }

    uint64 GetLastChange() const { return _lastChange; }
    void UpdateLastChange() { _lastChange = uint64(time(nullptr)); }

    uint32 GenerateTicketId() { return ++_lastTicketId; }
    uint32 GetOpenTicketCount() const { return _openTicketCount; }
    uint32 GetNextSurveyID() { return ++_lastSurveyId; }

    void Initialize();
    void ResetTickets();

    void ShowList(ChatHandler& handler, bool onlineOnly) const;
    void ShowClosedList(ChatHandler& handler) const;
    void ShowEscalatedList(ChatHandler& handler) const;

protected:
    GmTicketList _ticketList;

    bool   _status;
    uint32 _lastTicketId;
    uint32 _lastSurveyId;
    uint32 _openTicketCount;
    uint64 _lastChange;
};

#define sTicketMgr TicketMgr::instance()

#endif // _TICKETMGR_H
