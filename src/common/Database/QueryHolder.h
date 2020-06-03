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

#ifndef _QUERYHOLDER_H
#define _QUERYHOLDER_H

#include "DatabaseEnvFwd.h"

class SQLQueryHolder
{
    friend class SQLQueryHolderTask;
    typedef std::pair<SQLElementData, SQLResultSetUnion> SQLResultPair;
    std::vector<SQLResultPair> m_queries;
public:
    SQLQueryHolder() {}
    ~SQLQueryHolder();
    bool SetQuery(size_t index, const char *sql);
    bool SetPreparedQuery(size_t index, PreparedStatement* stmt);
    void SetSize(size_t size);
    QueryResult GetResult(size_t index);
    PreparedQueryResult GetPreparedResult(size_t index);
    void SetResult(size_t index, ResultSet* result);
    void SetPreparedResult(size_t index, PreparedResultSet* result);
};

class SQLQueryHolderTask : public SQLOperation
{
    SQLQueryHolder* m_holder;
    QueryResultHolderPromise m_result;
    bool m_executed;

public:
    SQLQueryHolderTask(SQLQueryHolder* holder);

    ~SQLQueryHolderTask();

    bool Execute() override;
    QueryResultHolderFuture GetFuture();
};

#endif
