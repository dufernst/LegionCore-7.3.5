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

#ifndef _ADHOCSTATEMENT_H
#define _ADHOCSTATEMENT_H

#include "SQLOperation.h"
#include "DatabaseEnvFwd.h"

class BasicStatementTask : public SQLOperation
{
public:
    BasicStatementTask(const char* sql, bool async = false, bool iscallback = false, std::function<void(QueryResult)> && callback = [](QueryResult) -> void {});
    ~BasicStatementTask();

    bool Execute() override;
    QueryResultFuture GetFuture() const;

private:
    const char* m_sql;      //- Raw query to be executed
    bool m_has_result;
    bool m_has_callback;
    QueryResultPromise* m_result;
    std::function<void(QueryResult)> m_Callback;
};

#endif
