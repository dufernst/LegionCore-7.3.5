#ifndef NACL_H
#define NACL_H

#include <random>

extern "C" {
	// for tweetnacl
	#include "tweetnacl.h"

    void randombytes(unsigned char* x, unsigned long long xlen)
    {
        std::random_device rd;
        std::uniform_int_distribution<unsigned int> dist(
            std::numeric_limits<unsigned int>::min(),
            std::numeric_limits<unsigned int>::max());

        while (xlen > 0)
        {
            unsigned int number = dist(rd);
            for (size_t i = sizeof(unsigned int); i > 0 ; i--)
            {
                unsigned char byteValue = (number >> (i - 1) * 8) & 0xFF;
                *x = byteValue;

                x += 1;
                xlen -= 1;

                if (xlen <= 0)
                    break;
            }
        }
    }
}

#endif
