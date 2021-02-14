/* crc32c.c -- compute CRC-32C using the Intel crc32 instruction
 * Copyright (C) 2013 Mark Adler
 * Version 1.1  1 Aug 2013  Mark Adler
 */

/*
  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the author be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Mark Adler
  madler@alumni.caltech.edu
 */

/* Use hardware CRC instruction on Intel SSE 4.2 processors.  This computes a
   CRC-32C, *not* the CRC-32 used by Ethernet and zip, gzip, etc.  A software
   version is provided as a fall-back, as well as for speed comparisons. */

/* Version history:
   1.0  10 Feb 2013  First version
   1.1   1 Aug 2013  Correct comments on why three crc instructions in parallel
   1.2         2020  Added gcc intrinsics, fixed alignment issues.
 */

#include "sc_crc32.h"

#include <stddef.h>

/* CRC-32C (iSCSI) polynomial in reversed bit order. */
#define CRC32_POLY 0x82f63b78

#ifdef HAVE_CRC32C
#include <memory.h>
#include <x86intrin.h>

/* Multiply a matrix times a vector over the Galois field of two elements,
   GF(2).  Each element is a bit in an unsigned integer.  mat must have at
   least as many entries as the power of two for most significant one bit in
   vec. */
static inline uint32_t gf2_matrix_times(uint32_t *mat, uint32_t vec)
{
    uint32_t sum = 0;

    while (vec) {
        if (vec & 1) {
            sum ^= *mat;
        }

        vec >>= 1;
        mat++;
    }

    return sum;
}

/* Multiply a matrix by itself over GF(2).  Both mat and square must have 32
   rows. */
static inline void gf2_matrix_square(uint32_t *square, uint32_t *mat)
{
    for (int n = 0; n < 32; n++) {
        square[n] = gf2_matrix_times(mat, mat[n]);
    }
}

/* Construct an operator to apply len zeros to a crc.  len must be a power of
   two.  If len is not a power of two, then the result is the same as for the
   largest power of two less than len.  The result for len == 0 is the same as
   for len == 1.  A version of this routine could be easily written for any
   len, but that is not needed for this application. */
static void crc32_zeros_op(uint32_t *even, size_t len)
{
    uint32_t row = 1;
    uint32_t odd[32]; /* odd-power-of-two zeros operator */

    /* put operator for one zero bit in odd */
    odd[0] = CRC32_POLY; /* CRC-32C polynomial */

    for (int n = 1; n < 32; n++) {
        odd[n] = row;
        row <<= 1;
    }

    /* put operator for two zero bits in even */
    gf2_matrix_square(even, odd);

    /* put operator for four zero bits in odd */
    gf2_matrix_square(odd, even);

    /* first square will put the operator for one zero byte (eight zero bits),
       in even -- next square puts operator for two zero bytes in odd, and so
       on, until len has been rotated down to zero */
    do {
        gf2_matrix_square(even, odd);
        len >>= 1;
        if (len == 0) {
            return;
        }
        gf2_matrix_square(odd, even);
        len >>= 1;
    } while (len);

    /* answer ended up in odd -- copy to even */
    for (int n = 0; n < 32; n++) {
        even[n] = odd[n];
    }
}

/* Take a length and build four lookup tables for applying the zeros operator
   for that length, byte-by-byte on the operand. */
static void crc32_zeros(uint32_t zeros[][256], size_t len)
{
    uint32_t op[32];

    crc32_zeros_op(op, len);

    for (uint32_t n = 0; n < 256; n++) {
        zeros[0][n] = gf2_matrix_times(op, n);
        zeros[1][n] = gf2_matrix_times(op, n << 8);
        zeros[2][n] = gf2_matrix_times(op, n << 16);
        zeros[3][n] = gf2_matrix_times(op, n << 24);
    }
}

/* Apply the zeros operator table to crc. */
static inline uint32_t crc32_shift(uint32_t zeros[][256], uint32_t crc)
{
    return zeros[0][(crc >> 0) & 0xff] ^ zeros[1][(crc >> 8) & 0xff] ^
           zeros[2][(crc >> 16) & 0xff] ^ zeros[3][(crc >> 24) & 0xff];
}

/* Block sizes for three-way parallel crc computation.  LONG and SHORT must
   both be powers of two.  The associated string constants must be set
   accordingly, for use in constructing the assembler instructions. */
#define CRC32_LONG  2048
#define CRC32_SHORT 256

static uint32_t crc32c_long[4][256];
static uint32_t crc32c_short[4][256];

static void crc32_init_hw(void)
{
    crc32_zeros(crc32c_long, CRC32_LONG);
    crc32_zeros(crc32c_short, CRC32_SHORT);
}

uint32_t crc32_hw(uint32_t crc, const uint8_t *buf, uint32_t len)
{
    const unsigned char *next = buf;
    const unsigned char *end;
    uint64_t crc0, crc1, crc2; /* need to be 64 bits for crc32q */

    /* pre-process the crc */
    crc0 = crc ^ 0xffffffff;

    /* compute the crc for up to seven leading bytes to bring the data pointer
       to an eight-byte boundary */
    while (len && ((uintptr_t) next & 7) != 0) {
        crc0 = _mm_crc32_u8(crc0, *next);
        next++;
        len--;
    }

    /* compute the crc on sets of LONG*3 bytes, executing three independent crc
       instructions, each on LONG bytes -- this is optimized for the Nehalem,
       Westmere, Sandy Bridge, and Ivy Bridge architectures, which have a
       throughput of one crc per cycle, but a latency of three cycles */
    while (len >= CRC32_LONG * 3) {
        crc1 = 0;
        crc2 = 0;

        end = next + CRC32_LONG;
        do {
            uint64_t a, b, c;

            memcpy(&a, next, 8);
            memcpy(&b, next + CRC32_LONG, 8);
            memcpy(&c, next + (CRC32_LONG * 2), 8);

            crc0 = _mm_crc32_u64(crc0, a);
            crc1 = _mm_crc32_u64(crc1, b);
            crc2 = _mm_crc32_u64(crc2, c);

            next += 8;
        } while (next < end);

        crc0 = crc32_shift(crc32c_long, crc0) ^ crc1;
        crc0 = crc32_shift(crc32c_long, crc0) ^ crc2;

        next += (CRC32_LONG * 2);
        len -= (CRC32_LONG * 3);
    }

    /* do the same thing, but now on SHORT * 3 blocks for the remaining data
       less than a LONG * 3 block */
    while (len >= CRC32_SHORT * 3) {
        crc1 = 0;
        crc2 = 0;

        end = next + CRC32_SHORT;
        do {
            uint64_t a, b, c;

            memcpy(&a, next, 8);
            memcpy(&b, next + CRC32_SHORT, 8);
            memcpy(&c, next + (CRC32_SHORT * 2), 8);

            crc0 = _mm_crc32_u64(crc0, a);
            crc1 = _mm_crc32_u64(crc1, b);
            crc2 = _mm_crc32_u64(crc2, c);

            next += 8;
        } while (next < end);

        crc0 = crc32_shift(crc32c_short, crc0) ^ crc1;
        crc0 = crc32_shift(crc32c_short, crc0) ^ crc2;

        next += (CRC32_SHORT * 2);
        len -= (CRC32_SHORT * 3);
    }

    /* compute the crc on the remaining eight-byte units less than a SHORT * 3
       block */
    end = next + (len - (len & 7));
    while (next < end) {
        uint64_t a;

        memcpy(&a, next, 8);
        crc0 = _mm_crc32_u64(crc0, a);
        next += 8;
    }
    len &= 7;

    /* compute the crc for up to seven trailing bytes */
    while (len) {
        crc0 = _mm_crc32_u8(crc0, *next);
        next++;
        len--;
    }

    /* return a post-processed crc */
    return (uint32_t) crc0 ^ 0xffffffff;
}

#else

/* Table for a quadword-at-a-time software crc. */
static uint32_t crc32c_table[8][256];

/* Construct table for software CRC-32C calculation. */
static void crc32_init_sw(void)
{
    for (uint32_t n = 0; n < 256; n++) {
        uint32_t crc = n;

        crc = crc & 1 ? (crc >> 1) ^ CRC32_POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ CRC32_POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ CRC32_POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ CRC32_POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ CRC32_POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ CRC32_POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ CRC32_POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ CRC32_POLY : crc >> 1;

        crc32c_table[0][n] = crc;
    }

    for (uint32_t n = 0; n < 256; n++) {
        uint32_t crc = crc32c_table[0][n];

        for (uint32_t k = 1; k < 8; k++) {
            crc = crc32c_table[0][crc & 0xff] ^ (crc >> 8);
            crc32c_table[k][n] = crc;
        }
    }
}

/* Table-driven software version as a fall-back.  This is about 15 times slower
   than using the hardware instructions.  This assumes little-endian integers,
   as is the case on Intel processors that the assembler code here is for. */
static uint32_t crc32_sw(uint32_t crci, const void *buf, size_t len)
{
    const unsigned char *next = buf;
    uint64_t crc = crci ^ 0xffffffff;

    while (len && ((uintptr_t) next & 7) != 0) {
        crc = crc32c_table[0][(crc ^ *next++) & 0xff] ^ (crc >> 8);
        len--;
    }

    while (len >= 8) {
        crc ^= *(uint64_t *) next;
        crc = crc32c_table[7][crc & 0xff] ^ crc32c_table[6][(crc >> 8) & 0xff] ^
              crc32c_table[5][(crc >> 16) & 0xff] ^
              crc32c_table[4][(crc >> 24) & 0xff] ^
              crc32c_table[3][(crc >> 32) & 0xff] ^
              crc32c_table[2][(crc >> 40) & 0xff] ^
              crc32c_table[1][(crc >> 48) & 0xff] ^ crc32c_table[0][crc >> 56];
        next += 8;
        len -= 8;
    }

    while (len) {
        crc = crc32c_table[0][(crc ^ *next++) & 0xff] ^ (crc >> 8);
        len--;
    }

    return (uint32_t) crc ^ 0xffffffff;
}

#endif

uint32_t sc_crc32(uint32_t crc, const uint8_t *buf, uint32_t len)
{
#ifdef HAVE_CRC32C
    return crc32_hw(crc, buf, len);
#else
    return crc32_sw(crc, buf, len);
#endif
}

void sc_crc32_init()
{
#ifdef HAVE_CRC32C
    crc32_init_hw();
#else
    crc32_init_sw();
#endif
}
