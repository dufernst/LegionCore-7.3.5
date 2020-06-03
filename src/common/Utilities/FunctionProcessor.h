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

#ifndef __FunctionProcessor_H
#define __FunctionProcessor_H

#include "Define.h"
#include <map>

typedef std::multimap<uint64, std::function<void()>> FunctionList;

class FunctionProcessor
{
    public:
        FunctionProcessor();
        ~FunctionProcessor();

        void Update(uint32 p_time);
        void KillAllFunctions();
        void AddFunction(std::function<void()> && Function, uint64 e_time);
        void AddFunctionsFromQueue();
        uint64 CalculateTime(uint64 t_offset) const;
        bool Empty() const;
        uint32 Size() const;
        uint32 SizeQueue() const;
        void AddDelayedEvent(uint64 t_offset, std::function<void()>&& function);

    protected:
        std::atomic<uint64> m_time;
        FunctionList m_functions;
        FunctionList m_functions_queue;
        std::recursive_mutex m_queue_lock;
        bool clean;
};
#endif