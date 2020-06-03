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

#ifndef _TRANSACTION_H
#define _TRANSACTION_H

#include "SQLOperation.h"
#include "DatabaseEnvFwd.h"
#include "StringFormat.h"

class PreparedStatement;

class Transaction
{
    friend class TransactionTask;
    friend class MySQLConnection;
    friend class DatabaseWokerPool;

    template <typename T>
    friend class DatabaseWorkerPool;

public:
    Transaction();
    ~Transaction();

    void Append(PreparedStatement* statement);
    void Append(const char* sql);
    template<typename Format, typename... Args>
    void PAppend(Format&& sql, Args&&... args)
    {
        Append(Trinity::StringFormat(std::forward<Format>(sql), std::forward<Args>(args)...).c_str());
    }

    size_t GetSize() const;

protected:
    void Cleanup();
    std::list<SQLElementData> m_queries;

private:
    bool _cleanedUp;
};

class TransactionTask : public SQLOperation
{
    template <class T> friend class DatabaseWorkerPool;
    friend class DatabaseWorker;

public:
    TransactionTask(SQLTransaction trans, std::function<void()>&& callback = []() -> void {});
    ~TransactionTask() = default;

protected:
    bool Execute() override;

    SQLTransaction m_trans;
    std::function<void()> m_Callback;
};

#endif
