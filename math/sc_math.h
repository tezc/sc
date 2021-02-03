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
#ifndef SC_MATH_H
#define SC_MATH_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define sc_math_max(a, b) (((a) > (b)) ? (a) : (b))
#define sc_math_min(a, b) (((a) > (b)) ? (b) : (a))

/**
 * @param num num
 * @return    'true' if num is power of 2
 */
bool sc_math_is_pow2(size_t num);

/**
 * @param size size
 * @return     next nearest power of two of size
 */
size_t sc_math_to_pow2(size_t size);

/**
 * Bytes to human readable form, e.g 10240 bytes  to 10 KB.
 *
 * @param buf  buf to write output
 * @param max  buf size
 * @param val  val to be converted into human readable form
 * @return     'buf' on success, 'NULL' on failure.
 */
char *sc_math_bytes_to_size(char *buf, size_t max, uint64_t val);

/**
 * Human readable string to bytes, e.g 10 KB to 10240 bytes.
 *
 * @param buf buf to write output
 * @return    positive value on success, '-1' on error
 */
int64_t sc_math_size_to_bytes(const char *buf);

#endif
