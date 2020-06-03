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

#include "DatabaseEnv.h"
#include "Transaction.h"
#include <mysqld_error.h>

//- Append a raw ad-hoc query to the transaction
void Transaction::Append(const char* sql)
{
    SQLElementData data;
    data.type = SQL_ELEMENT_RAW;
    data.element.query = strdup(sql);
    m_queries.push_back(data);
}

size_t Transaction::GetSize() const
{
    return m_queries.size();
}

Transaction::Transaction(): _cleanedUp(false)
{
}

//- Append a prepared statement to the transaction
Transaction::~Transaction()
{
    Cleanup();
}

void Transaction::Append(PreparedStatement* stmt)
{
    SQLElementData data;
    data.type = SQL_ELEMENT_PREPARED;
    data.element.stmt = stmt;
    m_queries.push_back(data);
}

void Transaction::Cleanup()
{
    // This might be called by explicit calls to Cleanup or by the auto-destructor
    if (_cleanedUp)
        return;

    while (!m_queries.empty())
    {
        SQLElementData const &data = m_queries.front();
        switch (data.type)
        {
            case SQL_ELEMENT_PREPARED:
                delete data.element.stmt;
            break;
            case SQL_ELEMENT_RAW:
                free((void*)(data.element.query));
            break;
        }

        m_queries.pop_front();
    }

    _cleanedUp = true;
}

TransactionTask::TransactionTask(SQLTransaction trans, std::function<void()>&& callback): m_trans(trans), m_Callback(std::move(callback))
{
}

bool TransactionTask::Execute()
{
    int errorCode = m_conn->ExecuteTransaction(m_trans);
    if (!errorCode)
    {
        m_Callback(); // Run lambda
        return true;
    }
    else if (errorCode == -1)
    {
        m_Callback(); // Run lambda again
        return true;
    }

    if (errorCode == ER_LOCK_DEADLOCK)
    {
        uint8 loopBreaker = 5;  // Handle MySQL Errno 1213 without extending deadlock to the core itself
        for (uint8 i = 0; i < loopBreaker; ++i)
        {
            if (!m_conn->ExecuteTransaction(m_trans))
            {
                m_Callback(); // Run lambda
                return true;
            }
        }
    }

    // Clean up now.
    m_trans->Cleanup();

    return false;
}
