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
#ifndef SC_TIME_H
#define SC_TIME_H

#include <stdint.h>

/**
 * This is not a monotonic timer.
 * @return Current timestamp in milliseconds.
 */
uint64_t sc_time_ms();

/**
 * This is not a monotonic timer.
 * @return Current timestamp in nanoseconds.
 */
uint64_t sc_time_ns();

/**
 * Monotonic timer.
 * @return Current timestamp in milliseconds.
 */
uint64_t sc_time_mono_ms();

/**
 * Monotonic timer.
 * @return Current timestamp in nanoseconds.
 */
uint64_t sc_time_mono_ns();

/**
 * @param milliseconds Milliseconds to sleep.
 * @return '0' on success, negative number on failure.
 */
int sc_time_sleep(uint64_t milliseconds);

#endif
