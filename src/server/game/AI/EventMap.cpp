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

#include "EventMap.h"

EventMap::EventMap() : _time(0), _phase(0) { }

uint32 EventMap::GetTimer() const
{
    return _time;
}

void EventMap::Reset()
{
    clear();
    _time = 0;
    _phase = 0;
}

void EventMap::Update(uint32 time)
{
    _time += time;
}

uint32 EventMap::GetPhaseMask() const
{
    return _phase;
}

bool EventMap::IsInPhase(uint8 phase)
{
    return phase <= 8 && (!phase || _phase & (1 << (phase - 1)));
}

bool EventMap::Empty() const
{
    return empty();
}

void EventMap::SetPhase(uint32 phase)
{
    if (!phase)
        _phase = 0;
    else if (phase <= 8)
        _phase = (1 << (phase - 1));
}

void EventMap::AddPhase(uint8 phase)
{
    if (phase && phase <= 8)
        _phase |= (1 << (phase - 1));
}

void EventMap::RemovePhase(uint8 phase)
{
    if (phase && phase <= 8)
        _phase &= ~(1 << (phase - 1));
}

void EventMap::ScheduleEvent(uint32 eventId, Seconds time, uint32 group, uint32 phase)
{
    ScheduleEvent(eventId, time.count(), group, phase);
}

void EventMap::ScheduleEvent(uint32 eventId, Minutes time, uint32 group, uint32 phase)
{
    ScheduleEvent(eventId, time.count(), group, phase);
}

void EventMap::ScheduleEvent(uint32 eventId, uint32 time, uint32 group, uint32 phase)
{
    uint32 data = 0;
    if (group && group <= 8)
        data |= (1 << (group + 15));
    if (phase && phase <= 8)
        data |= (1 << (phase + 23));
    time += _time;
    const_iterator itr = find(time);
    while (itr != end())
    {
        ++time;
        itr = find(time);
    }

    insert(std::make_pair(time, MAKE_PAIR64(eventId, data)));
}

void EventMap::RescheduleEvent(uint32 eventId, uint32 time, uint32 groupId, uint32 phase)
{
    CancelEvent(eventId);
    ScheduleEvent(eventId, time, groupId, phase);
}

bool EventMap::HasEvent(uint32 eventId) const
{
    for (auto itr = cbegin(); itr != cend(); ++itr)
        if ((itr->second & 0x0000FFFF) == eventId)
            return true;

    return false;
}

uint32 EventMap::GetEventTime(uint32 eventId) const
{
    for (auto itr = cbegin(); itr != cend(); ++itr)
        if ((itr->second & 0x0000FFFF) == eventId)
            return itr->first;

    return 0;
}

void EventMap::RepeatEvent(uint32 time)
{
    if (empty())
        return;

    auto eventData = begin()->second;
    erase(begin());
    time += _time;
    const_iterator itr = find(time);
    while (itr != end())
    {
        ++time;
        itr = find(time);
    }
    insert(std::make_pair(time, eventData));
}

void EventMap::PopEvent()
{
    erase(begin());
}

uint32 EventMap::ExecuteEvent()
{
    while (!empty())
    {
        if (begin()->first > _time)
            return 0;

        auto data = PAIR64_HIPART(begin()->second);
        if (_phase && (data & 0xFF000000) && !((data >> 24) & _phase))
            erase(begin());
        else
        {
            auto eventId = PAIR64_LOPART(begin()->second);
            erase(begin());
            return eventId;
        }
    }
    return 0;
}

uint32 EventMap::GetEvent()
{
    while (!empty())
    {
        if (begin()->first > _time)
            return 0;

        auto data = PAIR64_HIPART(begin()->second);
        if (_phase && (data & 0xFF000000) && !((data >> 24) & _phase))
            erase(begin());
        else
            return PAIR64_LOPART(begin()->second);
    }
    return 0;
}

void EventMap::DelayEvent(uint32 eventID, uint32 delay)
{
    auto nextTime = _time + delay;
    for (auto itr = begin(); itr != end() && itr->first < nextTime;)
    {
        if (PAIR64_LOPART(itr->second) == eventID)
        {
            auto data = PAIR64_HIPART(begin()->second);
            ScheduleEvent(eventID, itr->first - _time + delay, data >> 24, data >> 16);
            erase(itr);
            itr = begin();
        }
        else
            ++itr;
    }
}

void EventMap::DelayEvents(uint32 delay)
{
    if (delay < _time)
        _time -= delay;
    else
        _time = 0;
}

void EventMap::DelayEvents(uint32 delay, uint32 groupId)
{
    auto nextTime = _time + delay;
    uint32 groupMask = (1 << (groupId + 16));
    for (auto itr = begin(); itr != end() && itr->first < nextTime;)
    {
        auto data = PAIR64_HIPART(itr->second);
        if (data & groupMask)
        {
            ScheduleEvent(PAIR64_LOPART(itr->second), itr->first - _time + delay, data >> 24, data >> 16);
            erase(itr);
            itr = begin();
        }
        else
            ++itr;
    }
}

void EventMap::RecalcEventTimer(uint32 event, int32 minTime, bool dopTime)
{
    if (HasEvent(event))
    {
        int32 timer = GetNextEventTime(event) - GetTimer();
        if (timer < minTime)
            RescheduleEvent(event, dopTime ? minTime + timer : minTime);
    }
}

void EventMap::CancelEvent(uint32 eventId)
{
    for (auto itr = begin(); itr != end();)
    {
        if (eventId == PAIR64_LOPART(itr->second))
        {
            erase(itr);
            itr = begin();
        }
        else
            ++itr;
    }
}

void EventMap::CancelEventGroup(uint32 groupId)
{
    uint32 groupMask = (1 << (groupId + 16));
    for (auto itr = begin(); itr != end();)
    {
        auto data = PAIR64_HIPART(itr->second);
        if (data & groupMask)
        {
            erase(itr);
            itr = begin();
        }
        else
            ++itr;
    }
}

uint32 EventMap::GetNextEventTime(uint32 eventId) const
{
    for (auto itr = cbegin(); itr != cend(); ++itr)
        if (eventId == PAIR64_LOPART(itr->second))
            return itr->first;
    return 0;
}

