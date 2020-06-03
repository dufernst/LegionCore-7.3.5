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

#ifndef _PREPAREDSTATEMENT_H
#define _PREPAREDSTATEMENT_H

#include "SQLOperation.h"
#include "DatabaseEnvFwd.h"

//- Union for data buffer (upper-level bind -> queue -> lower-level bind)
union PreparedStatementDataUnion
{
    bool boolean;
    uint8 ui8;
    int8 i8;
    uint16 ui16;
    int16 i16;
    uint32 ui32;
    int32 i32;
    uint64 ui64;
    int64 i64;
    float f;
    double d;
};

//- This enum helps us differ data held in above union
enum PreparedStatementValueType
{
    TYPE_BOOL,
    TYPE_UI8,
    TYPE_UI16,
    TYPE_UI32,
    TYPE_UI64,
    TYPE_I8,
    TYPE_I16,
    TYPE_I32,
    TYPE_I64,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_STRING,
    TYPE_BINARY,
    TYPE_NULL
};

struct PreparedStatementData
{
    PreparedStatementDataUnion data;
    PreparedStatementValueType type;
    std::vector<uint8> binary;
};

//- Forward declare
class MySQLPreparedStatement;

//- Upper-level class that is used in code
class PreparedStatement
{
    friend class PreparedStatementTask;
    friend class MySQLPreparedStatement;
    friend class MySQLConnection;

    public:
        explicit PreparedStatement(uint32 index, uint8 capacity);
        ~PreparedStatement();

        void setBool(uint8 index, bool value);
        void setUInt8(uint8 index, uint8 value);
        void setUInt16(uint8 index, uint16 value);
        void setUInt32(uint8 index, uint32 value);
        void setUInt64(uint8 index, uint64 value);
        void setInt8(uint8 index, int8 value);
        void setInt16(uint8 index, int16 value);
        void setInt32(uint8 index, int32 value);
        void setInt64(uint8 index, int64 value);
        void setFloat(uint8 index, float value);
        void setDouble(uint8 index, double value);
        void setString(uint8 index, const std::string& value);
        void setBinary(uint8 index, const std::vector<uint8>& value);
        void setNull(uint8 index);

    protected:
        //- Copy the parameters to a real MySQLPreparedStatement
        void BindParameters(MySQLPreparedStatement* m_stmt) const;

        uint32 m_index;
        //- Buffer of parameters, not tied to MySQL in any way yet
        std::vector<PreparedStatementData> statement_data;
};

//- Class of which the instances are unique per MySQLConnection
//- access to these class objects is only done when a prepared statement task
//- is executed.
class MySQLPreparedStatement
{
    friend class MySQLConnection;
    friend class PreparedStatement;

    public:
        MySQLPreparedStatement(MYSQL_STMT* stmt);
        ~MySQLPreparedStatement();

        void setBool(uint8 index, bool value);
        void setUInt8(uint8 index, uint8 value);
        void setUInt16(uint8 index, uint16 value);
        void setUInt32(uint8 index, uint32 value);
        void setUInt64(uint8 index, uint64 value);
        void setInt8(uint8 index, int8 value);
        void setInt16(uint8 index, int16 value);
        void setInt32(uint8 index, int32 value);
        void setInt64(uint8 index, int64 value);
        void setFloat(uint8 index, float value);
        void setDouble(uint8 index, double value);
        void setBinary(uint8 index, const std::vector<uint8>& value, bool isString);
        void setNull(uint8 index);

    protected:
        MYSQL_STMT* GetSTMT() { return m_Mstmt; }
        MYSQL_BIND* GetBind() { return m_bind; }
        PreparedStatement* m_stmt;
        void ClearParameters();
        void AssertValidIndex(uint8 index);
        std::string getQueryString(std::string const &sqlPattern) const;

    private:
        void setValue(MYSQL_BIND* param, enum_field_types type, const void* value, uint32 len, bool isUnsigned);

        MYSQL_STMT* m_Mstmt;
        uint32 m_paramCount;
        std::vector<bool> m_paramsSet;
        MYSQL_BIND* m_bind;
        std::recursive_mutex i_data_lock;

        MySQLPreparedStatement(MySQLPreparedStatement const& right) = delete;
        MySQLPreparedStatement& operator=(MySQLPreparedStatement const& right) = delete;
};

//- Lower-level class, enqueuable operation
class PreparedStatementTask : public SQLOperation
{
    public:
        PreparedStatementTask(PreparedStatement* stmt, bool async = false, bool iscallback = false, std::function<void(PreparedQueryResult)> && callback = [](PreparedQueryResult) -> void {});
        ~PreparedStatementTask();

        bool Execute() override;
        PreparedQueryResultFuture GetFuture() { return m_result->get_future(); }

    protected:
        PreparedStatement* m_stmt;
        bool m_has_result;
        bool m_has_callback;
        PreparedQueryResultPromise* m_result;
        std::function<void(PreparedQueryResult)> m_Callback;
};
#endif
