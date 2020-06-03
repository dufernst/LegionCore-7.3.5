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

#ifndef _FIELD_H
#define _FIELD_H

#include "Common.h"
#include "Log.h"

#include <mysql.h>

class Field
{
    friend class ResultSet;
    friend class PreparedResultSet;

public:
    bool GetBool() const; // Wrapper, actually gets integer
    uint8 GetUInt8() const;
    int8 GetInt8() const;
    uint16 GetUInt16() const;
    int16 GetInt16() const;
    uint32 GetUInt32() const;
    int32 GetInt32() const;
    uint64 GetUInt64() const;
    int64 GetInt64() const;
    float GetFloat() const;
    double GetDouble() const;
    char const* GetCString() const;
    std::string GetString() const;
    std::vector<uint8> GetBinary() const;
    bool IsNull() const;

protected:
    Field();
    ~Field();

#if defined(__GNUC__)
#pragma pack(1)
#else
#pragma pack(push, 1)
#endif
    struct
    {
        uint32 length;          // Length (prepared strings only)
        void* value;            // Actual data in memory
        enum_field_types type;  // Field type
        bool raw;               // Raw bytes? (Prepared statement or ad hoc)
    } data;
#if defined(__GNUC__)
#pragma pack()
#else
#pragma pack(pop)
#endif

    void SetByteValue(void const* newValue, size_t newSize, enum_field_types newType, uint32 length);
    void SetStructuredValue(char* newValue, enum_field_types newType, uint32 length);

    void CleanUp();

    static size_t SizeForType(MYSQL_FIELD* field);

    bool IsType(enum_field_types type) const;
    bool IsNumeric() const;

private:
#ifdef TRINITY_DEBUG
    static char const* FieldTypeToString(enum_field_types type)
    {
        switch (type)
        {
        case MYSQL_TYPE_BIT:         return "BIT";
        case MYSQL_TYPE_BLOB:        return "BLOB";
        case MYSQL_TYPE_DATE:        return "DATE";
        case MYSQL_TYPE_DATETIME:    return "DATETIME";
        case MYSQL_TYPE_NEWDECIMAL:  return "NEWDECIMAL";
        case MYSQL_TYPE_DECIMAL:     return "DECIMAL";
        case MYSQL_TYPE_DOUBLE:      return "DOUBLE";
        case MYSQL_TYPE_ENUM:        return "ENUM";
        case MYSQL_TYPE_FLOAT:       return "FLOAT";
        case MYSQL_TYPE_GEOMETRY:    return "GEOMETRY";
        case MYSQL_TYPE_INT24:       return "INT24";
        case MYSQL_TYPE_LONG:        return "LONG";
        case MYSQL_TYPE_LONGLONG:    return "LONGLONG";
        case MYSQL_TYPE_LONG_BLOB:   return "LONG_BLOB";
        case MYSQL_TYPE_MEDIUM_BLOB: return "MEDIUM_BLOB";
        case MYSQL_TYPE_NEWDATE:     return "NEWDATE";
        case MYSQL_TYPE_NULL:        return "NULL";
        case MYSQL_TYPE_SET:         return "SET";
        case MYSQL_TYPE_SHORT:       return "SHORT";
        case MYSQL_TYPE_STRING:      return "STRING";
        case MYSQL_TYPE_TIME:        return "TIME";
        case MYSQL_TYPE_TIMESTAMP:   return "TIMESTAMP";
        case MYSQL_TYPE_TINY:        return "TINY";
        case MYSQL_TYPE_TINY_BLOB:   return "TINY_BLOB";
        case MYSQL_TYPE_VAR_STRING:  return "VAR_STRING";
        case MYSQL_TYPE_YEAR:        return "YEAR";
        default:                     return "-Unknown-";
        }
    }
#endif
};

#endif

