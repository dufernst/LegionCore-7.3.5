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

#include "Field.h"

bool Field::GetBool() const
{
    return GetUInt8() == 1 ? true : false;
}

uint8 Field::GetUInt8() const
{
    if (!data.value)
        return 0;

#ifdef TRINITY_DEBUG
    if (!IsNumeric())
    {
        TC_LOG_WARN(LOG_FILTER_SQL, "Warning: GetUInt8() on non-tinyint field. Using type: %s.", FieldTypeToString(data.type));
        return 0;
    }
#endif

    if (data.raw)
        return *reinterpret_cast<uint8*>(data.value);
    return static_cast<uint8>(strtoul(static_cast<char*>(data.value), nullptr, 10));
}

int8 Field::GetInt8() const
{
    if (!data.value)
        return 0;

#ifdef TRINITY_DEBUG
    if (!IsNumeric())
    {
        TC_LOG_WARN(LOG_FILTER_SQL, "Warning: GetInt8() on non-tinyint field. Using type: %s.", FieldTypeToString(data.type));
        return 0;
    }
#endif

    if (data.raw)
        return *reinterpret_cast<int8*>(data.value);
    return static_cast<int8>(strtol(static_cast<char*>(data.value), NULL, 10));
}

uint16 Field::GetUInt16() const
{
    if (!data.value)
        return 0;

#ifdef TRINITY_DEBUG
    if (!IsNumeric())
    {
        TC_LOG_WARN(LOG_FILTER_SQL, "Warning: GetUInt16() on non-smallint field. Using type: %s.", FieldTypeToString(data.type));
        return 0;
    }
#endif

    if (data.raw)
        return *reinterpret_cast<uint16*>(data.value);
    return static_cast<uint16>(strtoul(static_cast<char*>(data.value), nullptr, 10));
}

int16 Field::GetInt16() const
{
    if (!data.value)
        return 0;

#ifdef TRINITY_DEBUG
    if (!IsNumeric())
    {
        TC_LOG_WARN(LOG_FILTER_SQL, "Warning: GetInt16() on non-smallint field. Using type: %s.", FieldTypeToString(data.type));
        return 0;
    }
#endif

    if (data.raw)
        return *reinterpret_cast<int16*>(data.value);
    return static_cast<int16>(strtol(static_cast<char*>(data.value), NULL, 10));
}

uint32 Field::GetUInt32() const
{
    if (!data.value)
        return 0;

#ifdef TRINITY_DEBUG
    if (!IsNumeric())
    {
        TC_LOG_WARN(LOG_FILTER_SQL, "Warning: GetUInt32() on non-(medium)int field. Using type: %s.", FieldTypeToString(data.type));
        return 0;
    }
#endif

    if (data.raw)
        return *reinterpret_cast<uint32*>(data.value);
    return static_cast<uint32>(strtoul(static_cast<char*>(data.value), nullptr, 10));
}

int32 Field::GetInt32() const
{
    if (!data.value)
        return 0;

#ifdef TRINITY_DEBUG
    if (!IsNumeric())
    {
        TC_LOG_WARN(LOG_FILTER_SQL, "Warning: GetInt32() on non-(medium)int field. Using type: %s.", FieldTypeToString(data.type));
        return 0;
    }
#endif

    if (data.raw)
        return *reinterpret_cast<int32*>(data.value);
    return static_cast<int32>(strtol(static_cast<char*>(data.value), NULL, 10));
}

uint64 Field::GetUInt64() const
{
    if (!data.value)
        return 0;

#ifdef TRINITY_DEBUG
    if (!IsNumeric())
    {
        TC_LOG_WARN(LOG_FILTER_SQL, "Warning: GetUInt64() on non-bigint field. Using type: %s.", FieldTypeToString(data.type));
        return 0;
    }
#endif

    if (data.raw)
        return *reinterpret_cast<uint64*>(data.value);
    return static_cast<uint64>(strtoull(static_cast<char*>(data.value), nullptr, 10));
}

int64 Field::GetInt64() const
{
    if (!data.value)
        return 0;

#ifdef TRINITY_DEBUG
    if (!IsNumeric())
    {
        TC_LOG_WARN(LOG_FILTER_SQL, "Warning: GetInt64() on non-bigint field. Using type: %s.", FieldTypeToString(data.type));
        return 0;
    }
#endif

    if (data.raw)
        return *reinterpret_cast<int64*>(data.value);
    return static_cast<int64>(strtoll(static_cast<char*>(data.value), NULL, 10));
}

float Field::GetFloat() const
{
    if (!data.value)
        return 0.0f;

#ifdef TRINITY_DEBUG
    if (!IsNumeric())
    {
        TC_LOG_WARN(LOG_FILTER_SQL, "Warning: GetFloat() on non-float field. Using type: %s.", FieldTypeToString(data.type));
        return 0.0f;
    }
#endif

    if (data.raw)
        return *reinterpret_cast<float*>(data.value);
    return static_cast<float>(atof(static_cast<char*>(data.value)));
}

double Field::GetDouble() const
{
    if (!data.value)
        return 0.0f;

#ifdef TRINITY_DEBUG
    if (!IsNumeric())
    {
        TC_LOG_WARN(LOG_FILTER_SQL, "Warning: GetDouble() on non-double field. Using type: %s.", FieldTypeToString(data.type));
        return 0.0f;
    }
#endif

    if (data.raw)
        return *reinterpret_cast<double*>(data.value);
    return static_cast<double>(atof(static_cast<char*>(data.value)));
}

char const* Field::GetCString() const
{
    if (!data.value)
        return NULL;

#ifdef TRINITY_DEBUG
    if (IsNumeric())
    {
        TC_LOG_WARN(LOG_FILTER_SQL, "Error: GetCString() on numeric field. Using type: %s.", FieldTypeToString(data.type));
        return NULL;
    }
#endif
    return static_cast<char const*>(data.value);
}

std::string Field::GetString() const
{
    if (!data.value)
        return "";

    char const* string = GetCString();
    if (!string)
        return "";

    return std::string(string, data.length);
}

std::vector<uint8> Field::GetBinary() const
{
    std::vector<uint8> result;
    if (!data.value || !data.length)
        return result;

    result.resize(data.length);
    memcpy(result.data(), data.value, data.length);
    return result;
}

bool Field::IsNull() const
{
    return data.value == NULL;
}

Field::Field()
{
    data.value = NULL;
    data.type = MYSQL_TYPE_NULL;
    data.length = 0;
    data.raw = false;
}

Field::~Field()
{
    CleanUp();
}

void Field::SetByteValue(const void* newValue, const size_t newSize, enum_field_types newType, uint32 length)
{
    if (data.value)
        CleanUp();

    // This value stores raw bytes that have to be explicitly cast later
    if (newValue)
    {
        data.value = new char[newSize];
        memcpy(data.value, newValue, newSize);
        data.length = length;
    }
    data.type = newType;
    data.raw = true;
}

void Field::SetStructuredValue(char* newValue, enum_field_types newType, uint32 length)
{
    if (data.value)
        CleanUp();

    // This value stores somewhat structured data that needs function style casting
    if (newValue)
    {
        data.value = new char[length + 1];
        memcpy(data.value, newValue, length);
        *(reinterpret_cast<char*>(data.value) + length) = '\0';
        data.length = length;
    }

    data.type = newType;
    data.raw = false;
}

void Field::CleanUp()
{
    delete[]static_cast<char*>(data.value);
    data.value = NULL;
}

size_t Field::SizeForType(MYSQL_FIELD* field)
{
    switch (field->type)
    {
    case MYSQL_TYPE_NULL:
        return 0;
    case MYSQL_TYPE_TINY:
        return 1;
    case MYSQL_TYPE_YEAR:
    case MYSQL_TYPE_SHORT:
        return 2;
    case MYSQL_TYPE_INT24:
    case MYSQL_TYPE_LONG:
    case MYSQL_TYPE_FLOAT:
        return 4;
    case MYSQL_TYPE_DOUBLE:
    case MYSQL_TYPE_LONGLONG:
    case MYSQL_TYPE_BIT:
        return 8;

    case MYSQL_TYPE_TIMESTAMP:
    case MYSQL_TYPE_DATE:
    case MYSQL_TYPE_TIME:
    case MYSQL_TYPE_DATETIME:
        return sizeof(MYSQL_TIME);

    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_BLOB:
    case MYSQL_TYPE_STRING:
    case MYSQL_TYPE_VAR_STRING:
        return field->max_length + 1;

    case MYSQL_TYPE_DECIMAL:
    case MYSQL_TYPE_NEWDECIMAL:
        return 64;

    case MYSQL_TYPE_GEOMETRY:
        /*
        Following types are not sent over the wire:
        MYSQL_TYPE_ENUM:
        MYSQL_TYPE_SET:
        */
    default:
        TC_LOG_WARN(LOG_FILTER_SQL, "SQL::SizeForType(): invalid field type %u", uint32(field->type));
        return 0;
    }
}

bool Field::IsType(enum_field_types type) const
{
    return data.type == type;
}

bool Field::IsNumeric() const
{
    return (data.type == MYSQL_TYPE_TINY ||
        data.type == MYSQL_TYPE_SHORT ||
        data.type == MYSQL_TYPE_INT24 ||
        data.type == MYSQL_TYPE_LONG ||
        data.type == MYSQL_TYPE_FLOAT ||
        data.type == MYSQL_TYPE_DOUBLE ||
        data.type == MYSQL_TYPE_LONGLONG);
}
