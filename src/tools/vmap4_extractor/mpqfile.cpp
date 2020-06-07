#include "mpqfile.h"
#include <deque>
#include <cstdio>

MPQFile::MPQFile(const char* filename) :
    eof(false),
    buffer(0),
    pointer(0),
    size(0)
{
    FILE* f = fopen(filename, "rb");
    if (!f)
    {
        // fprintf(stderr, "Can't open %s\n", filename);
        eof = true;
        return;
    }

    fseek(f, 0, SEEK_END);
    size_t sizeTemp = ftell(f);
    size = sizeTemp;
    rewind(f);

    if (size <= 1)
    {
        fprintf(stderr, "Can't open %s, size = %u!\n", filename, uint32(size));
        fclose(f);
        eof = true;
        return;
    }

    buffer = new char[size];
    if (fread(buffer, sizeTemp, 1, f) != 1)
    {
        fprintf(stderr, "Can't read %s, size=%u!\n", filename, uint32(size));
        fclose(f);
        eof = true;
        return;
    }
    size = sizeTemp;

    fclose(f);
}

size_t MPQFile::read(void* dest, size_t bytes)
{
    if (eof) return 0;

    size_t rpos = pointer + bytes;
    if (rpos > size) {
        bytes = size - pointer;
        eof = true;
    }

    memcpy(dest, &(buffer[pointer]), bytes);

    pointer = rpos;

    return bytes;
}

void MPQFile::seek(int offset)
{
    pointer = offset;
    eof = (pointer >= size);
}

void MPQFile::seekRelative(int offset)
{
    pointer += offset;
    eof = (pointer >= size);
}

void MPQFile::close()
{
    if (buffer) delete[] buffer;
    buffer = 0;
    eof = true;
}
