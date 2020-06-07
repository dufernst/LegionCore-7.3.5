/*
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
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

#ifndef DBCFILE_H
#define DBCFILE_H
#include <cassert>
#include <string>
#include <list>

#include "Define.h"

class DBCFile
{
public:
    DBCFile(char const* file, char const* fmt);
    ~DBCFile();

    bool open();

    class Iterator;
    class Record
    {
    public:
        float getFloat(size_t field) const;
        uint32 getUInt(size_t field) const;
        uint8 getUInt8(size_t field) const;
        uint16 getUInt16(size_t field) const;
        uint64 getUInt64(size_t field) const;
        char const* getString(size_t field) const;

    private:
        Record(DBCFile& file, unsigned char* offset) : file(file), offset(offset)  { }
        DBCFile& file;
        unsigned char* offset;

        friend class DBCFile;
        friend class DBCFile::Iterator;

        Record& operator=(Record const& right);
    };

    class Iterator
    {
    public:
        Iterator(DBCFile &file, unsigned char* offset) : record(file, offset) { }
        Iterator& operator++()
        {
            record.offset += record.file.header.RecordSize;
            return *this;
        }

        Record const& operator*() const {  return record; }
        Record const* operator->() const { return &record; }
        bool operator==(Iterator const& b) const { return record.offset == b.record.offset; }
        bool operator!=(Iterator const& b) const { return record.offset != b.record.offset; }

    private:
        Record record;
        Iterator& operator=(Iterator const& right);
    };

    Record getRecord(size_t id);
    uint32 GetOffset(size_t id) const { return (fieldsOffset != nullptr && id < header.FieldCount) ? fieldsOffset[id] : 0; }
    Iterator begin();
    Iterator end();
    size_t getRecordCount() const { return header.RecordCount; }
    size_t getFieldCount() const { return header.FieldCount; }
    size_t getMaxId();

private:
    uint32* fieldsOffset;
    char const* _file;
    unsigned char* recordTable;
    unsigned char* stringTable;
    char const* _fmt;

    struct
    {
        uint32 Signature;
        uint32 RecordCount;
        uint32 FieldCount;
        uint32 RecordSize;
        uint32 BlockValue;
        uint32 Hash;
        uint32 Build;
        uint32 TimeStamp;
        uint32 Min;
        uint32 Max;
        uint32 Locale;
        uint32 ReferenceDataSize;
        uint32 MetaFlags;
    } header;
};

#endif
