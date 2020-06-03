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

#include "FunctionProcessor.h"

FunctionProcessor::FunctionProcessor()
{
    m_time = 0;
    clean = false;
}

FunctionProcessor::~FunctionProcessor()
{
}

void FunctionProcessor::Update(uint32 p_time)
{
    //move from queue
    AddFunctionsFromQueue();

    // update time
    m_time += p_time;

    if (clean)
    {
        m_functions.clear();
        clean = false;
        return;
    }

    if (m_functions.empty())
        return;

    // main event loop
    FunctionList::iterator i;
    while (((i = m_functions.begin()) != m_functions.end()) && i->first <= m_time)
    {
        // get and remove event from queue
        i->second();
        m_functions.erase(i);
    }
}

void FunctionProcessor::KillAllFunctions()
{
    clean = true;
}

void FunctionProcessor::AddFunction(std::function<void()> && Function, uint64 e_time)
{
    std::lock_guard<std::recursive_mutex> _queue_lock(m_queue_lock);
    m_functions_queue.insert(std::make_pair(e_time, Function));
}

void FunctionProcessor::AddFunctionsFromQueue()
{
    if (m_functions_queue.empty())
        return;

    FunctionList tempFunctions;
    {
        std::lock_guard<std::recursive_mutex> _queue_lock(m_queue_lock);
        std::swap(tempFunctions, m_functions_queue);
    }
    FunctionList::iterator itr = tempFunctions.begin();
    for(; itr != tempFunctions.end(); ++itr)
        m_functions.insert(std::make_pair(itr->first, itr->second));
}

uint64 FunctionProcessor::CalculateTime(uint64 t_offset) const
{
    return m_time + t_offset;
}

bool FunctionProcessor::Empty() const
{
    return m_functions.empty();
}

uint32 FunctionProcessor::Size() const
{
    return m_functions.size();
}

uint32 FunctionProcessor::SizeQueue() const
{
    return m_functions_queue.size();
}

void FunctionProcessor::AddDelayedEvent(uint64 t_offset, std::function<void()>&& function)
{
    AddFunction(std::move(function), m_time + t_offset);
}

