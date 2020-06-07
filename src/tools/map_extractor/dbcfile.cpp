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

#define _CRT_SECURE_NO_DEPRECATE

#include "dbcfile.h"

DBCFile::DBCFile(char const* file, char const* fmt)
{
    _file = file;
    _fmt = fmt;
    recordTable = nullptr;
    stringTable = nullptr;
    fieldsOffset = nullptr;

    header.RecordSize = 0;
    header.RecordCount = 0;
    header.FieldCount = 0;
    header.BlockValue = 0;
    header.Signature = 0;
    header.Hash = 0;
    header.Build = 0;
    header.TimeStamp = 0;
    header.Min = 0;
    header.Max = 0;
    header.Locale = 0;
    header.ReferenceDataSize = 0;
    header.MetaFlags = 0;
}

bool DBCFile::open()
{
    if (recordTable)
    {
        delete[] recordTable;
        recordTable = nullptr;
    }

    FILE* f = fopen(_file, "rb");
    if (!f)
        return false;

    if (fread(&header.Signature, sizeof(uint32), 1, f) != 1)
    {
        fclose(f);
        return false;
    }

    if (fread(&header.RecordCount, sizeof(uint32), 1, f) != 1)
    {
        fclose(f);
        return false;
    }

    if (fread(&header.FieldCount, sizeof(uint32), 1, f) != 1)
    {
        fclose(f);
        return false;
    }

    if (fread(&header.RecordSize, sizeof(uint32), 1, f) != 1)
    {
        fclose(f);
        return false;
    }

    if (fread(&header.BlockValue, sizeof(uint32), 1, f) != 1)
    {
        fclose(f);
        return false;
    }

    fread(&header.Hash, sizeof(uint32), 1, f);
    fread(&header.Build, sizeof(uint32), 1, f);
    fread(&header.TimeStamp, sizeof(uint32), 1, f);
    fread(&header.Min, sizeof(uint32), 1, f);
    fread(&header.Max, sizeof(uint32), 1, f);
    fread(&header.Locale, sizeof(uint32), 1, f);
    fread(&header.ReferenceDataSize, sizeof(uint32), 1, f);
    //fread(&header.MetaFlags, sizeof(uint32), 1, f);

    recordTable = new unsigned char[header.RecordSize * header.RecordCount + header.BlockValue];
    stringTable = recordTable + header.RecordSize * header.RecordCount;

    if (fread(recordTable, header.RecordSize * header.RecordCount + header.BlockValue, 1, f) != 1)
    {
        fclose(f);
        return false;
    }

    fieldsOffset = new uint32[header.FieldCount];
    fieldsOffset[0] = 0;
    for (uint32 i = 1; i < header.FieldCount; i++)
    {
        fieldsOffset[i] = fieldsOffset[i - 1];
        switch (_fmt[i - 1])
        {
            case 'l':
                fieldsOffset[i] += sizeof(uint64);
                break;
            case 'n':
            case 'i':
            case 'f':
                fieldsOffset[i] += sizeof(uint32);
                break;
            case 't':
                fieldsOffset[i] += sizeof(uint16);
                break;
            case 'b':
                fieldsOffset[i] += sizeof(uint8);
                break;
            case 's':
                fieldsOffset[i] += 4/*sizeof(char*)*/; //! WARNING! Size 4
                break;
            default:
                break;

        }
    }

    fclose(f);
    return true;
}

DBCFile::~DBCFile()
{
    delete[] recordTable;
}

DBCFile::Record DBCFile::getRecord(size_t id)
{
    assert(recordTable);
    return Record(*this, recordTable + id * header.RecordSize);
}

float DBCFile::Record::getFloat(size_t field) const
{
    assert(field < file.header.FieldCount);
    return *reinterpret_cast<float*>(offset + file.GetOffset(field));
}

uint32 DBCFile::Record::getUInt(size_t field) const
{
    assert(field < file.header.FieldCount);
    return *reinterpret_cast<uint32*>(offset + file.GetOffset(field));
}

uint8 DBCFile::Record::getUInt8(size_t field) const
{
    assert(field < file.header.FieldCount);
    return *reinterpret_cast<uint8*>(offset + file.GetOffset(field));
}

uint16 DBCFile::Record::getUInt16(size_t field) const
{
    assert(field < file.header.FieldCount);
    return *reinterpret_cast<uint16*>(offset + file.GetOffset(field));
}

uint64 DBCFile::Record::getUInt64(size_t field) const
{
    assert(field < file.header.FieldCount);
    return *reinterpret_cast<uint64*>(offset + file.GetOffset(field));
}

char const* DBCFile::Record::getString(size_t field) const
{
    assert(field < file.header.FieldCount);
    return reinterpret_cast<char*>(file.stringTable + getUInt(field));
}

size_t DBCFile::getMaxId()
{
    assert(recordTable);

    size_t maxId = 0;
    for (size_t i = 0; i < getRecordCount(); ++i)
        if (maxId < getRecord(i).getUInt(0))
            maxId = getRecord(i).getUInt(0);

    return maxId;
}

DBCFile::Iterator DBCFile::begin()
{
    assert(recordTable);
    return Iterator(*this, recordTable);
}

DBCFile::Iterator DBCFile::end()
{
    assert(recordTable);
    return Iterator(*this, stringTable);
}
