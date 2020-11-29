/*
 * MIT License
 *
 * Copyright (c) 2020 Ozan Tezcan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "sc_math.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if (SIZE_MAX == 0xFFFF)
    #define SIZE_T_BITS 16
#elif (SIZE_MAX == 0xFFFFFFFF)
    #define SIZE_T_BITS 32
#elif (SIZE_MAX == 0xFFFFFFFFFFFFFFFF)
    #define SIZE_T_BITS 64
#else
    #error unknown size_t bits
#endif

bool sc_math_is_pow2(size_t num)
{
    return (num != 0) && (num & (num - 1)) == 0;
}

size_t sc_math_to_pow2(size_t size)
{
    if (size == 0) {
        return 1;
    }

    size--;

    for (uint32_t i = 1; i < sizeof(size) * 8; i *= 2) {
        size |= size >> i;
    }

    size++;

    return size;
}

char *sc_math_bytes_to_size(char *buf, size_t max, uint64_t val)
{
    int i, size;
    uint64_t bytes = 1;
    char *suffix[] = {"B", "KB", "MB", "GB", "TB", "PB"};
    size_t length = sizeof(suffix) / sizeof(suffix[0]);

    for (i = 0; bytes * 1024 < val && i < length; i++) {
        bytes *= 1024;
    }

    size = snprintf(buf, max, "%.02lf %s", (((double) val) / bytes), suffix[i]);
    if (size <= 0 || size >= max) {
        return NULL;
    }

    return buf;
}

int64_t sc_math_size_to_bytes(const char *buf)
{
    int count;
    int64_t val;
    char *parse_end;
    char *end = (char *) (buf + strlen(buf));

    errno = 0;
    val = strtoll(buf, &parse_end, 10);
    if (errno != 0 || parse_end == buf) {
        return -1;
    }

    count = (int) (end - parse_end);

    switch (count) {
    case 0:
        return val;
    case 1:
        break;
    case 2:
        if (tolower(parse_end[1]) != 'b') {
            return -1;
        }
        break;
    default:
        return -1;
    }

    switch (tolower(parse_end[0])) {
    case 'b':
        break;
    case 'k':
        val *= 1024;
        break;
    case 'm':
        val *= 1024 * 1024;
        break;
    case 'g':
        val *= 1024 * 1024 * 1024;
        break;
    case 'p':
        val *= 1024 * 1024 * 1024 * 1024ull;
        break;
    default:
        return -1;
    }

    return val;
}
